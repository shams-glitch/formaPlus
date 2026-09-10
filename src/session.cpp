#include "session.h"

Session& Session::instance()
{
    static Session s;
    return s;
}

void Session::definir(int id, const QString& login, Role role)
{
    id_ = id;
    login_ = login;
    role_ = role;
    authentifie_ = (role == Role::Admin || role == Role::Formateur);
}

void Session::vider()
{
    id_ = 0;
    login_.clear();
    role_ = Role::Inconnu;
    authentifie_ = false;
}

QString Session::libelleRole() const
{
    if (role_ == Role::Admin) {
        return QString::fromUtf8("Admin");
    }
    if (role_ == Role::Formateur) {
        return QString::fromUtf8("Formateur");
    }
    return QString::fromUtf8("Inconnu");
}

Session::Role Session::roleDepuisTexte(const QString& texte)
{
    const QString t = texte.trimmed().toUpper();
    if (t == QLatin1String("ADMIN")) {
        return Role::Admin;
    }
    if (t == QLatin1String("FORMATEUR")) {
        return Role::Formateur;
    }
    return Role::Inconnu;
}

QString Session::texteDepuisRole(Role role)
{
    if (role == Role::Admin) {
        return QStringLiteral("ADMIN");
    }
    if (role == Role::Formateur) {
        return QStringLiteral("FORMATEUR");
    }
    return QString();
}

bool Session::exigerAdmin(QString* erreur)
{
    if (instance().estAdmin()) {
        return true;
    }
    if (erreur) {
        *erreur = QString::fromUtf8("Action réservée à l'administrateur.");
    }
    return false;
}
