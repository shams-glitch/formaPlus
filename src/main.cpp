#include "gcentreformation.h"
#include "connection.h"
#include "authentification.h"
#include "login.h"
#include "session.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QString::fromUtf8("GCentreFormation"));
    a.setOrganizationName(QString::fromUtf8("FormaPlus"));
    a.setQuitOnLastWindowClosed(false);

    Connection* c = Connection::instance();
    const bool test = c->createConnect();
    if (!test) {
        QMessageBox::critical(nullptr,
                              QString::fromUtf8("Connexion Oracle"),
                              QString::fromUtf8(
                                  "Connexion échouée.\n\n"
                                  "Vérifiez :\n"
                                  "1. Oracle XE est démarré\n"
                                  "2. La source ODBC s'appelle Source_Projet2A\n"
                                  "3. Variables ORACLE_USER / ORACLE_PASSWORD, ou connection.ini à côté de l'exe\n"
                                  "4. Le script sql/mpd_oracle.sql a été exécuté"));
        return 1;
    }

    Authentification auth;
    if (!auth.preparer()) {
        QMessageBox::critical(nullptr,
                              QString::fromUtf8("Authentification"),
                              auth.lastError());
        return 1;
    }

    LoginDialog login;
    GCentreFormation w;
    QObject::connect(&w, &GCentreFormation::deconnexionDemandee, [&]() {
        w.hide();
        Session::instance().vider();
        login.reinitialiser();
        if (login.exec() != QDialog::Accepted) {
            a.quit();
        } else {
            w.appliquerProfil();
            w.show();
        }
    });

    if (login.exec() != QDialog::Accepted) {
        return 0;
    }
    w.appliquerProfil();
    w.show();
    return a.exec();
}
