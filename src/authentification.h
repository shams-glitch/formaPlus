#ifndef AUTHENTIFICATION_H
#define AUTHENTIFICATION_H

#include "session.h"

#include <QString>

class Authentification
{
public:
    bool preparer();
    bool authentifier(const QString& login, const QString& motDePasse);
    QString lastError() const { return lastError_; }

private:
    QString lastError_;

    bool assurerTable();
    bool assurerColonneRole();
    bool bootstrapComptesLocaux();
    bool compteExiste(const QString& login, bool& existe);
    bool insererCompte(const QString& login, const QString& motDePasse, Session::Role role);
    QByteArray hacher(const QByteArray& motDePasse, const QByteArray& sel) const;
};

#endif // AUTHENTIFICATION_H
