#ifndef AUTHENTIFICATION_H
#define AUTHENTIFICATION_H

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
    bool compterUtilisateurs(int& nb);
    bool bootstrapLocalSiVide();
    QByteArray hacher(const QByteArray& motDePasse, const QByteArray& sel) const;
};

#endif // AUTHENTIFICATION_H
