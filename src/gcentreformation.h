#ifndef GCENTREFORMATION_H
#define GCENTREFORMATION_H

#include <QMainWindow>
#include <QModelIndex>

#include "formateur.h"
#include "cours.h"

class QSqlQueryModel;
class QTableView;
class QComboBox;

QT_BEGIN_NAMESPACE
namespace Ui { class GCentreFormation; }
QT_END_NAMESPACE

class GCentreFormation : public QMainWindow
{
    Q_OBJECT

public:
    explicit GCentreFormation(QWidget *parent = nullptr);
    ~GCentreFormation();

private slots:
    void on_btnNavAccueil_clicked();
    void on_btnNavFormateurs_clicked();
    void on_btnNavCours_clicked();

    void on_btnFormAjouter_clicked();
    void on_btnFormModifier_clicked();
    void on_btnFormSupprimer_clicked();
    void on_btnFormVider_clicked();
    void on_btnFormActualiser_clicked();
    void on_btnFormPdf_clicked();
    void on_tableFormateurs_clicked(const QModelIndex& index);

    void on_btnFormRechercher_clicked();
    void on_btnFormTrier_clicked();
    void on_btnFormPdfListe_clicked();

    void on_btnFormRefreshMetiers_clicked();

    void on_btnCoursAjouter_clicked();
    void on_btnCoursModifier_clicked();
    void on_btnCoursSupprimer_clicked();
    void on_btnCoursVider_clicked();
    void on_btnCoursActualiser_clicked();
    void on_btnCoursPdf_clicked();
    void on_btnCoursCatalogue_clicked();
    void on_tableCours_clicked(const QModelIndex& index);

    void on_btnCoursRechercher_clicked();
    void on_btnCoursTrier_clicked();

    void on_btnCoursAffecter_clicked();
    void on_btnCoursRefreshAlertes_clicked();

private:
    Ui::GCentreFormation *ui;
    Formateur tmpFormateur;
    Cours tmpCours;

    void appliquerStyle();
    void configurerControles();
    void chargerTout();
    void chargerTableFormateurs();
    void chargerTableCours();
    void chargerCombosFormateurs();
    void remplirComboDepuisModele(QComboBox* combo, QSqlQueryModel* model, bool avecTous);
    void appliquerModele(QTableView* table, QSqlQueryModel* model);
    void viderFormulaireFormateur();
    void viderFormulaireCours();
    Formateur formateurDepuisFormulaire() const;
    Cours coursDepuisFormulaire() const;
    void rafraichirStatistiques();
    void rafraichirAccueil();
    QString choisirFichierPdf(const QString& nomDefaut);
    void informer(bool ok, const QString& succes, const QString& erreur);
};

#endif // GCENTREFORMATION_H
