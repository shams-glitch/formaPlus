#include "connection.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QDebug>
#include <QTextStream>

Connection* Connection::p_instance = nullptr;

Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection* Connection::instance()
{
    if (p_instance == nullptr) {
        p_instance = new Connection();
    }
    return p_instance;
}

static void ecrireDiag(const QString& ligne)
{
    const QString chemin = QDir(QCoreApplication::applicationDirPath())
            .filePath(QStringLiteral("connection_error.log"));
    QFile f(chemin);
    if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        out << ligne << "\n";
    }
}

static void appliquerLigneIni(const QString& cle, const QString& val,
                              QString& dsn, QString& user, QString& password, QString& driver)
{
    if (cle == QLatin1String("dsn") && dsn.isEmpty()) {
        dsn = val;
    } else if (cle == QLatin1String("user") && user.isEmpty()) {
        user = val;
    } else if (cle == QLatin1String("password") && password.isEmpty()) {
        password = val;
    } else if (cle == QLatin1String("driver") && driver.isEmpty()) {
        driver = val;
    }
}

static void lireFichierIni(const QString& chemin, QString& dsn, QString& user, QString& password, QString& driver)
{
    QFile f(chemin);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
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
        appliquerLigneIni(cle, val, dsn, user, password, driver);
    }
}

static bool lireIdentifiants(QString& dsn, QString& user, QString& password, QString& driver)
{
    dsn = qEnvironmentVariable("ORACLE_DSN");
    user = qEnvironmentVariable("ORACLE_USER");
    password = qEnvironmentVariable("ORACLE_PASSWORD");
    driver = qEnvironmentVariable("ORACLE_ODBC_DRIVER");

    const QStringList candidats = {
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("connection.ini")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("connection.local.ini"))
    };
    for (const QString& chemin : candidats) {
        if (QFile::exists(chemin)) {
            lireFichierIni(chemin, dsn, user, password, driver);
        }
    }

    if (dsn.trimmed().isEmpty()) {
        dsn = QStringLiteral("Source_Projet2A");
    }
    if (driver.trimmed().isEmpty()) {
        driver = QStringLiteral("Oracle in XE");
    }
    return !user.trimmed().isEmpty() && !password.isEmpty();
}

bool Connection::createConnect()
{
    QString dsn;
    QString user;
    QString password;
    QString driver;
    const bool credsOk = lireIdentifiants(dsn, user, password, driver);
    ecrireDiag(QStringLiteral("dir=") + QCoreApplication::applicationDirPath()
               + QStringLiteral(" dsn=") + dsn
               + QStringLiteral(" user=") + (user.isEmpty() ? QStringLiteral("(vide)") : QStringLiteral("(fourni)"))
               + QStringLiteral(" password=") + (password.isEmpty() ? QStringLiteral("(vide)") : QStringLiteral("(fourni)")));

    db.setDatabaseName(dsn);
    if (credsOk) {
        db.setUserName(user);
        db.setPassword(password);
    }

    if (db.open()) {
        qDebug() << "Connexion a la base de donnees reussie";
        return true;
    }

    ecrireDiag(QStringLiteral("echec DSN: ") + db.lastError().text());

    if (credsOk) {
        db.close();
        db.setDatabaseName(QString("DRIVER={%1};DSN=%2;UID=%3;PWD=%4")
                           .arg(driver, dsn, user, password));
        db.setUserName(QString());
        db.setPassword(QString());
        if (db.open()) {
            qDebug() << "Connexion a la base de donnees reussie (DSN+UID)";
            return true;
        }
        ecrireDiag(QStringLiteral("echec DRIVER+DSN: ") + db.lastError().text());

        db.close();
        db.setDatabaseName(QString("DRIVER={%1};Dbq=localhost/XE;Uid=%2;Pwd=%3")
                           .arg(driver, user, password));
        if (db.open()) {
            qDebug() << "Connexion a la base de donnees reussie (DRIVER+Dbq)";
            return true;
        }
        ecrireDiag(QStringLiteral("echec DRIVER+Dbq: ") + db.lastError().text());
    }

    return false;
}

void Connection::closeConnection()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool Connection::isOpen() const
{
    return db.isOpen();
}

QSqlDatabase Connection::database() const
{
    return db;
}

Connection::~Connection()
{
    closeConnection();
}
