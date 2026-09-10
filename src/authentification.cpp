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
#include <QVector>

namespace {

struct CompteLocal {
    QString login;
    QString motDePasse;
    Session::Role role = Session::Role::Inconnu;
};

bool oracleCode(const QSqlQuery& q, const char* code)
{
    return q.lastError().text().contains(QLatin1String(code));
}

QVector<CompteLocal> lireComptesLocaux()
{
    const QStringList candidats = {
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("app_auth.local.ini")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("app_auth.ini"))
    };

    QVector<CompteLocal> comptes;
    for (const QString& chemin : candidats) {
        QFile f(chemin);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QTextStream in(&f);
        CompteLocal courant;
        QString section;
        auto flush = [&]() {
            if (!courant.login.trimmed().isEmpty() && !courant.motDePasse.isEmpty()) {
                if (courant.role == Session::Role::Inconnu) {
                    if (section.compare(QLatin1String("admin"), Qt::CaseInsensitive) == 0) {
                        courant.role = Session::Role::Admin;
                    } else {
                        courant.role = Session::Role::Formateur;
                    }
                }
                comptes.push_back(courant);
            }
            courant = CompteLocal();
        };

        while (!in.atEnd()) {
            QString ligne = in.readLine().trimmed();
            if (ligne.isEmpty() || ligne.startsWith(QLatin1Char('#'))) {
                continue;
            }
            if (ligne.startsWith(QLatin1Char('[')) && ligne.endsWith(QLatin1Char(']'))) {
                flush();
                section = ligne.mid(1, ligne.size() - 2).trimmed();
                continue;
            }
            const int eq = ligne.indexOf(QLatin1Char('='));
            if (eq <= 0) {
                continue;
            }
            const QString cle = ligne.left(eq).trimmed().toLower();
            const QString val = ligne.mid(eq + 1).trimmed();
            if (cle == QLatin1String("username") || cle == QLatin1String("login")) {
                courant.login = val;
            } else if (cle == QLatin1String("password") && courant.motDePasse.isEmpty()) {
                courant.motDePasse = val;
            } else if (cle == QLatin1String("role")) {
                courant.role = Session::roleDepuisTexte(val);
            }
        }
        flush();
        if (!comptes.isEmpty()) {
            break;
        }
    }
    return comptes;
}

} // namespace

bool Authentification::preparer()
{
    if (!assurerTable()) {
        return false;
    }
    if (!assurerColonneRole()) {
        return false;
    }
    return bootstrapComptesLocaux();
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
            "ROLE VARCHAR2(20) DEFAULT 'FORMATEUR' NOT NULL, "
            "CONSTRAINT CK_APP_UTILISATEUR_ACTIF CHECK (ACTIF IN (0,1)), "
            "CONSTRAINT CK_APP_UTILISATEUR_ROLE CHECK (ROLE IN ('ADMIN','FORMATEUR')))"))
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

bool Authentification::assurerColonneRole()
{
    QSqlQuery alter;
    if (!alter.exec(QStringLiteral("ALTER TABLE APP_UTILISATEUR ADD ROLE VARCHAR2(20)"))
            && !oracleCode(alter, "ORA-01430") && !oracleCode(alter, "01430")) {
        lastError_ = QString::fromUtf8("Impossible d'ajouter la colonne de rôle.");
        return false;
    }

    QSqlQuery maj;
    if (!maj.exec(QStringLiteral(
            "UPDATE APP_UTILISATEUR SET ROLE = 'FORMATEUR' "
            "WHERE ROLE IS NULL"))) {
        lastError_ = QString::fromUtf8("Impossible d'initialiser les rôles existants.");
        return false;
    }

    QSqlQuery ck;
    if (!ck.exec(QStringLiteral(
            "ALTER TABLE APP_UTILISATEUR ADD CONSTRAINT CK_APP_UTILISATEUR_ROLE "
            "CHECK (ROLE IN ('ADMIN','FORMATEUR'))"))
            && !oracleCode(ck, "ORA-02264") && !oracleCode(ck, "02264")
            && !oracleCode(ck, "ORA-02260") && !oracleCode(ck, "02260")
            && !oracleCode(ck, "ORA-02436") && !oracleCode(ck, "02436")) {
        lastError_ = QString::fromUtf8("Impossible d'ajouter la contrainte de rôle.");
        return false;
    }

    QSqlQuery nn;
    if (!nn.exec(QStringLiteral("ALTER TABLE APP_UTILISATEUR MODIFY ROLE VARCHAR2(20) NOT NULL"))
            && !oracleCode(nn, "ORA-01442") && !oracleCode(nn, "01442")
            && !oracleCode(nn, "ORA-01451") && !oracleCode(nn, "01451")) {
        lastError_ = QString::fromUtf8("Impossible de rendre le rôle obligatoire.");
        return false;
    }
    return true;
}

bool Authentification::compteExiste(const QString& login, bool& existe)
{
    QSqlQuery q;
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM APP_UTILISATEUR WHERE LOGIN = :login"));
    q.bindValue(QStringLiteral(":login"), login);
    if (!q.exec() || !q.next()) {
        lastError_ = QString::fromUtf8("Lecture des comptes applicatifs impossible.");
        return false;
    }
    existe = q.value(0).toInt() > 0;
    return true;
}

QByteArray Authentification::hacher(const QByteArray& motDePasse, const QByteArray& sel) const
{
    return QPasswordDigestor::deriveKeyPbkdf2(
                QCryptographicHash::Sha256, motDePasse, sel, 80000, 32);
}

bool Authentification::insererCompte(const QString& login, const QString& motDePasse, Session::Role role)
{
    QByteArray sel(16, 0);
    for (int i = 0; i < sel.size(); ++i) {
        sel[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    const QByteArray hash = hacher(motDePasse.toUtf8(), sel);

    QSqlQuery ins;
    ins.prepare(QStringLiteral(
        "INSERT INTO APP_UTILISATEUR (LOGIN, SEL, MOT_DE_PASSE_HASH, ACTIF, ROLE) "
        "VALUES (:login, :sel, :hash, 1, :role)"));
    ins.bindValue(QStringLiteral(":login"), login);
    ins.bindValue(QStringLiteral(":sel"), QString::fromLatin1(sel.toHex()));
    ins.bindValue(QStringLiteral(":hash"), QString::fromLatin1(hash.toHex()));
    ins.bindValue(QStringLiteral(":role"), Session::texteDepuisRole(role));
    if (!ins.exec()) {
        lastError_ = QString::fromUtf8("Création du compte local impossible.");
        return false;
    }
    return true;
}

bool Authentification::bootstrapComptesLocaux()
{
    const QVector<CompteLocal> comptes = lireComptesLocaux();
    if (comptes.isEmpty()) {
        QSqlQuery q;
        q.prepare(QStringLiteral("SELECT COUNT(*) FROM APP_UTILISATEUR"));
        if (!q.exec() || !q.next()) {
            lastError_ = QString::fromUtf8("Lecture des comptes applicatifs impossible.");
            return false;
        }
        if (q.value(0).toInt() > 0) {
            return true;
        }
        lastError_ = QString::fromUtf8(
            "Aucun compte applicatif. Créez app_auth.local.ini à côté de l'exe "
            "(voir app_auth.local.ini.example), puis relancez.");
        return false;
    }

    for (const CompteLocal& c : comptes) {
        const QString login = c.login.trimmed().toUpper();
        if (c.role == Session::Role::Inconnu) {
            lastError_ = QString::fromUtf8("Rôle local invalide (ADMIN ou FORMATEUR).");
            return false;
        }
        bool existe = false;
        if (!compteExiste(login, existe)) {
            return false;
        }
        if (existe) {
            continue;
        }
        if (!insererCompte(login, c.motDePasse, c.role)) {
            return false;
        }
    }
    return true;
}

bool Authentification::authentifier(const QString& login, const QString& motDePasse)
{
    Session::instance().vider();
    if (login.trimmed().isEmpty() || motDePasse.isEmpty()) {
        lastError_ = QString::fromUtf8("Nom d'utilisateur ou mot de passe incorrect.");
        return false;
    }

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT ID_UTILISATEUR, SEL, MOT_DE_PASSE_HASH, ROLE FROM APP_UTILISATEUR "
        "WHERE LOGIN = :login AND ACTIF = 1"));
    q.bindValue(QStringLiteral(":login"), login.trimmed().toUpper());
    if (!q.exec()) {
        lastError_ = QString::fromUtf8("Erreur d'authentification.");
        return false;
    }
    if (!q.next()) {
        lastError_ = QString::fromUtf8("Nom d'utilisateur ou mot de passe incorrect.");
        return false;
    }

    const int id = q.value(0).toInt();
    const QByteArray sel = QByteArray::fromHex(q.value(1).toString().toLatin1());
    const QByteArray attendu = QByteArray::fromHex(q.value(2).toString().toLatin1());
    const Session::Role role = Session::roleDepuisTexte(q.value(3).toString());
    const QByteArray calcule = hacher(motDePasse.toUtf8(), sel);
    if (sel.isEmpty() || attendu.isEmpty() || role == Session::Role::Inconnu || calcule != attendu) {
        lastError_ = QString::fromUtf8("Nom d'utilisateur ou mot de passe incorrect.");
        return false;
    }

    Session::instance().definir(id, login.trimmed().toUpper(), role);
    lastError_.clear();
    return true;
}
