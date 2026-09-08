#include "gcentreformation.h"
#include "connection.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QString::fromUtf8("GCentreFormation"));
    a.setOrganizationName(QString::fromUtf8("FormaPlus"));

    Connection* c = Connection::instance();
    const bool test = c->createConnect();

    GCentreFormation w;
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
    }
    w.show();
    return a.exec();
}
