#ifndef FORMATEUR_H
#define FORMATEUR_H

#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QMap>

class Formateur
{
public:
    Formateur();
    Formateur(int id, const QString& cin, const QString& nom, const QString& prenom,
              const QString& email, const QString& telephone, const QString& specialite,
              const QDate& dateEmbauche, const QString& statut);

    int getId() const { return id; }
    QString getCin() const { return cin; }
    QString getNom() const { return nom; }
    QString getPrenom() const { return prenom; }
    QString getEmail() const { return email; }
    QString getTelephone() const { return telephone; }
    QString getSpecialite() const { return specialite; }
    QDate getDateEmbauche() const { return dateEmbauche; }
    QString getStatut() const { return statut; }
    QString getLastError() const { return lastError; }

    bool valider(QString& message) const;
    bool ajouter();
    bool modifier();
    bool supprimer(int idFormateur);
    QSqlQueryModel* afficher();
    QSqlQueryModel* rechercher(const QString& nomCritere,
                               const QString& specialiteCritere,
                               const QString& statutCritere);
    QSqlQueryModel* trier(const QString& critere, const QString& ordre);
    QSqlQueryModel* trierMulti(const QStringList& criteres, const QStringList& ordres);
    QSqlQueryModel* listePourCombo();
    QMap<QString, int> statistiquesParSpecialite();
    QMap<QString, int> statistiquesParStatut();
    QSqlQueryModel* chargePedagogique();
    QSqlQueryModel* conflitsPlanning();
    int compter();
    int compterActifs();
    bool exporterFichePdf(int idFormateur, const QString& chemin);
    bool exporterListePdf(const QString& nomCritere,
                          const QString& specialiteCritere,
                          const QString& statutCritere,
                          const QString& chemin);

private:
    int id;
    QString cin;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString specialite;
    QDate dateEmbauche;
    QString statut;
    mutable QString lastError;

    void setOracleError(const QSqlQuery& query);
    QSqlQueryModel* executerSelect(QSqlQuery& query);
    QString htmlEntete(const QString& titre) const;
    QString htmlPied() const;
    bool ecrirePdf(const QString& html, const QString& chemin);
};

#endif // FORMATEUR_H
