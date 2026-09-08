#include "connection.h"
#include <QSqlError>
#include <QDebug>

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

bool Connection::createConnect()
{
    bool test = false;

    // DSN ODBC Windows (64 bits) + utilisateur Oracle du projet
    db.setDatabaseName("Source_Projet2A");
    db.setUserName("cpp");
    db.setPassword("cpp");

    if (db.open()) {
        test = true;
        qDebug() << "Connexion a la base de donnees reussie";
    } else {
        qDebug() << "Erreur de connexion:" << db.lastError().text();
    }

    return test;
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
