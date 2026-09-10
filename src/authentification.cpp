#include "authentification.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>

namespace {

bool lireAuthLocal(QString& login, QString& motDePasse)
{
    const QStringList candidats = {
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("app_auth.local.ini")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("app_auth.ini"))
    };
    for (const QString& chemin : candidats) {
        QFile f(chemin);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }
        QTextStream in(&f);
        while (!in.atEnd()) {
            QString ligne = in.readLine().trimmed();
            if (ligne.isEmpty() || ligne.startsWith(QLatin1Char('#')) || ligne.startsWith(QLatin1Char('['))) {
                continue;
            }
            const int eq = ligne.indexOf(QLatin1Char('='));
            if (eq <= 0) {
                continue;
            }
            const QString cle = ligne.left(eq).trimmed().toLower();
            const QString val = ligne.mid(eq + 1).trimmed();
            if (cle == QLatin1String("username") || cle == QLatin1String("login")) {
                login = val;
            } else if (cle == QLatin1String("password") && motDePasse.isEmpty()) {
                motDePasse = val;
            }
        }
        if (!login.trimmed().isEmpty() && !motDePasse.isEmpty()) {
            return true;
        }
    }
    return false;
}

} // namespace

bool Authentification::preparer()
{
    if (!assurerTable()) {
        return false;
    }
    return bootstrapLocalSiVide();
}

bool Authentification::assurerTable()
{
    auto existeDeja = [](const QSqlQuery& q) {
        const QString t = q.lastError().text();
        return t.contains(QLatin1String("ORA-00955")) || t.contains(QLatin1String("00955"));
    };

    QSqlQuery table;
    if (!table.exec(QStringLiteral(
            "CREATE TABLE APP_UTILISATEUR ("
            "ID_UTILISATEUR NUMBER CONSTRAINT PK_APP_UTILISATEUR PRIMARY KEY, "
            "LOGIN VARCHAR2(50) CONSTRAINT UQ_APP_UTILISATEUR_LOGIN UNIQUE NOT NULL, "
            "SEL VARCHAR2(64) NOT NULL, "
            "MOT_DE_PASSE_HASH VARCHAR2(128) NOT NULL, "
            "ACTIF NUMBER(1) DEFAULT 1 NOT NULL, "
            "CONSTRAINT CK_APP_UTILISATEUR_ACTIF CHECK (ACTIF IN (0,1)))"))
            && !existeDeja(table)) {
        lastError_ = QString::fromUtf8("Impossible de préparer la table d'authentification.");
        return false;
    }

    QSqlQuery seq;
    if (!seq.exec(QStringLiteral("CREATE SEQUENCE SEQ_APP_UTILISATEUR START WITH 1 NOCACHE"))
            && !existeDeja(seq)) {
        lastError_ = QString::fromUtf8("Impossible de préparer la séquence d'authentification.");
        return false;
    }

    QSqlQuery trg;
    if (!trg.exec(QStringLiteral(
            "CREATE OR REPLACE TRIGGER TRG_APP_UTILISATEUR_BI "
            "BEFORE INSERT ON APP_UTILISATEUR FOR EACH ROW "
            "BEGIN IF :NEW.ID_UTILISATEUR IS NULL THEN "
            "SELECT SEQ_APP_UTILISATEUR.NEXTVAL INTO :NEW.ID_UTILISATEUR FROM DUAL; "
            "END IF; END;"))) {
        lastError_ = QString::fromUtf8("Impossible de préparer le trigger d'authentification.");
        return false;
    }
    return true;
}

bool Authentification::compterUtilisateurs(int& nb)
{
    QSqlQuery q;
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM APP_UTILISATEUR"));
    if (!q.exec() || !q.next()) {
        lastError_ = QString::fromUtf8("Lecture des comptes applicatifs impossible.");
        return false;
    }
    nb = q.value(0).toInt();
    return true;
}

QByteArray Authentification::hacher(const QByteArray& motDePasse, const QByteArray& sel) const
{
    return QPasswordDigestor::deriveKeyPbkdf2(
                QCryptographicHash::Sha256, motDePasse, sel, 80000, 32);
}

bool Authentification::bootstrapLocalSiVide()
{
    int nb = 0;
    if (!compterUtilisateurs(nb)) {
        return false;
    }
    if (nb > 0) {
        return true;
    }

    QString login;
    QString motDePasse;
    if (!lireAuthLocal(login, motDePasse)) {
        lastError_ = QString::fromUtf8(
            "Aucun compte applicatif. Créez app_auth.local.ini à côté de l'exe "
            "(voir app_auth.local.ini.example), puis relancez.");
        return false;
    }

    QByteArray sel(16, 0);
    for (int i = 0; i < sel.size(); ++i) {
        sel[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    const QByteArray hash = hacher(motDePasse.toUtf8(), sel);

    QSqlQuery ins;
    ins.prepare(QStringLiteral(
        "INSERT INTO APP_UTILISATEUR (LOGIN, SEL, MOT_DE_PASSE_HASH, ACTIF) "
        "VALUES (:login, :sel, :hash, 1)"));
    ins.bindValue(QStringLiteral(":login"), login.trimmed().toUpper());
    ins.bindValue(QStringLiteral(":sel"), QString::fromLatin1(sel.toHex()));
    ins.bindValue(QStringLiteral(":hash"), QString::fromLatin1(hash.toHex()));
    if (!ins.exec()) {
        lastError_ = QString::fromUtf8("Création du compte local impossible.");
        return false;
    }
    return true;
}

bool Authentification::authentifier(const QString& login, const QString& motDePasse)
{
    if (login.trimmed().isEmpty() || motDePasse.isEmpty()) {
        lastError_ = QString::fromUtf8("Saisissez l'identifiant et le mot de passe.");
        return false;
    }

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT SEL, MOT_DE_PASSE_HASH FROM APP_UTILISATEUR "
        "WHERE LOGIN = :login AND ACTIF = 1"));
    q.bindValue(QStringLiteral(":login"), login.trimmed().toUpper());
    if (!q.exec()) {
        lastError_ = QString::fromUtf8("Erreur d'authentification.");
        return false;
    }
    if (!q.next()) {
        lastError_ = QString::fromUtf8("Identifiant ou mot de passe incorrect.");
        return false;
    }

    const QByteArray sel = QByteArray::fromHex(q.value(0).toString().toLatin1());
    const QByteArray attendu = QByteArray::fromHex(q.value(1).toString().toLatin1());
    const QByteArray calcule = hacher(motDePasse.toUtf8(), sel);
    if (sel.isEmpty() || attendu.isEmpty() || calcule != attendu) {
        lastError_ = QString::fromUtf8("Identifiant ou mot de passe incorrect.");
        return false;
    }
    lastError_.clear();
    return true;
}
