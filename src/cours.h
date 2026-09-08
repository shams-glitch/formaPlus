#ifndef COURS_H
#define COURS_H

#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QMap>

class Cours
{
public:
    Cours();
    Cours(int id, const QString& titre, const QString& niveau, int dureeH, double prix,
          const QDate& dateDebut, const QDate& dateFin, const QString& statut, int idFormateur);

    int getId() const { return id; }
    QString getTitre() const { return titre; }
    QString getNiveau() const { return niveau; }
    int getDureeH() const { return dureeH; }
    double getPrix() const { return prix; }
    QDate getDateDebut() const { return dateDebut; }
    QDate getDateFin() const { return dateFin; }
    QString getStatut() const { return statut; }
    int getIdFormateur() const { return idFormateur; }
    QString getLastError() const { return lastError; }

    bool valider(QString& message) const;
    bool ajouter();
    bool modifier();
    bool supprimer(int idCours);
    bool affecterFormateur(int idCours, int idFormateur);
    QSqlQueryModel* afficher();
    QSqlQueryModel* rechercher(const QString& titreCritere,
                               const QString& niveauCritere,
                               const QString& statutCritere,
                               int idFormateurCritere);
    QSqlQueryModel* trier(const QString& critere, const QString& ordre);
    QSqlQueryModel* trierMulti(const QStringList& criteres, const QStringList& ordres);
    QSqlQueryModel* listePourCombo();
    QMap<QString, int> statistiquesParNiveau();
    QMap<QString, int> statistiquesParFormateur();
    QMap<QString, int> statistiquesParMois();
    QSqlQueryModel* alertes();
    int compter();
    int compterParStatut(const QString& statutCible);
    double sommeHeures();
    bool exporterFichePdf(int idCours, const QString& chemin);
    bool exporterCataloguePdf(const QString& chemin);

private:
    int id;
    QString titre;
    QString niveau;
    int dureeH;
    double prix;
    QDate dateDebut;
    QDate dateFin;
    QString statut;
    int idFormateur;
    mutable QString lastError;

    void setOracleError(const QSqlQuery& query);
    QSqlQueryModel* executerSelect(QSqlQuery& query);
    QString htmlEntete(const QString& titreDoc) const;
    QString htmlPied() const;
    bool ecrirePdf(const QString& html, const QString& chemin);
};

#endif // COURS_H
