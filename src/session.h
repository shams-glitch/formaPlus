#ifndef SESSION_H
#define SESSION_H

#include <QString>

class Session
{
public:
    enum class Role {
        Inconnu,
        Admin,
        Formateur
    };

    static Session& instance();

    void definir(int id, const QString& login, Role role);
    void vider();

    bool estAuthentifie() const { return authentifie_; }
    bool estAdmin() const { return authentifie_ && role_ == Role::Admin; }
    Role role() const { return role_; }
    int id() const { return id_; }
    QString login() const { return login_; }
    QString libelleRole() const;

    static Role roleDepuisTexte(const QString& texte);
    static QString texteDepuisRole(Role role);
    static bool exigerAdmin(QString* erreur);

private:
    Session() = default;
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    int id_ = 0;
    QString login_;
    Role role_ = Role::Inconnu;
    bool authentifie_ = false;
};

#endif // SESSION_H
