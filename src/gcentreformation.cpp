#include "gcentreformation.h"
#include "ui_gcentreformation.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QHeaderView>
#include <QLayoutItem>
#include <QLayout>
#include <QPainter>
#include <QStatusBar>
#include <QButtonGroup>
#include <QDate>
#include <QAbstractItemModel>
#include <QTableView>
#include <QComboBox>
#include <QMap>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

namespace {

void remplirComboTriFormateur(QComboBox* cb, bool avecAucun)
{
    cb->clear();
    if (avecAucun) {
        cb->addItem(QString::fromUtf8("(aucun)"), QString());
    }
    cb->addItem(QString::fromUtf8("Nom"), QString("NOM"));
    cb->addItem(QString::fromUtf8("Prénom"), QString("PRENOM"));
    cb->addItem(QString::fromUtf8("Spécialité"), QString("SPECIALITE"));
    cb->addItem(QString::fromUtf8("Statut"), QString("STATUT"));
    cb->addItem(QString::fromUtf8("Date d'embauche"), QString("DATE_EMBAUCHE"));
    cb->addItem(QString::fromUtf8("ID"), QString("ID_FORMATEUR"));
}

void remplirComboTriCours(QComboBox* cb, bool avecAucun)
{
    cb->clear();
    if (avecAucun) {
        cb->addItem(QString::fromUtf8("(aucun)"), QString());
    }
    cb->addItem(QString::fromUtf8("Titre"), QString("TITRE"));
    cb->addItem(QString::fromUtf8("Niveau"), QString("NIVEAU"));
    cb->addItem(QString::fromUtf8("Durée"), QString("DUREE_H"));
    cb->addItem(QString::fromUtf8("Prix"), QString("PRIX"));
    cb->addItem(QString::fromUtf8("Date de début"), QString("DATE_DEBUT"));
    cb->addItem(QString::fromUtf8("Statut"), QString("STATUT"));
    cb->addItem(QString::fromUtf8("ID"), QString("ID_COURS"));
}

void remplirComboOrdreTri(QComboBox* cb)
{
    cb->clear();
    cb->addItem(QString::fromUtf8("Croissant"), QString("ASC"));
    cb->addItem(QString::fromUtf8("Décroissant"), QString("DESC"));
}

void collecterTri(QComboBox* critere, QComboBox* ordre,
                  QStringList& criteres, QStringList& ordres)
{
    const QString col = critere->currentData().toString();
    if (!col.isEmpty()) {
        criteres << col;
        ordres << ordre->currentData().toString();
    }
}

} // namespace

namespace {

void remplacerChart(QLayout* layout, QChart* chart)
{
    if (!layout) {
        delete chart;
        return;
    }
    while (QLayoutItem* item = layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    QChartView* view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(280);
    layout->addWidget(view);
}

QChart* creerCamembert(const QString& titre, const QMap<QString, int>& data)
{
    QPieSeries* series = new QPieSeries();
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        QPieSlice* slice = series->append(QString("%1 (%2)").arg(it.key()).arg(it.value()), it.value());
        slice->setLabelVisible(true);
    }
    if (series->count() == 0) {
        series->append(QString::fromUtf8("Aucune donnée"), 1);
    }
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(titre);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    return chart;
}

QChart* creerBarres(const QString& titre, const QMap<QString, int>& data)
{
    QBarSet* set = new QBarSet(QString::fromUtf8("Effectif"));
    QStringList categories;
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        *set << it.value();
        categories << it.key();
    }
    if (categories.isEmpty()) {
        *set << 0;
        categories << QString::fromUtf8("—");
    }
    QBarSeries* series = new QBarSeries();
    series->append(set);

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(titre);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setMin(0);
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    chart->legend()->setVisible(false);
    return chart;
}

} // namespace

GCentreFormation::GCentreFormation(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GCentreFormation)
{
    ui->setupUi(this);
    setWindowTitle(QString::fromUtf8("GCentreFormation — FormaPlus"));
    resize(1400, 860);

    QButtonGroup* nav = new QButtonGroup(this);
    nav->setExclusive(true);
    nav->addButton(ui->btnNavAccueil);
    nav->addButton(ui->btnNavFormateurs);
    nav->addButton(ui->btnNavCours);

    appliquerStyle();
    configurerControles();
    chargerTout();
}

GCentreFormation::~GCentreFormation()
{
    delete ui;
}

void GCentreFormation::appliquerStyle()
{
    setStyleSheet(QString::fromUtf8(
        "QMainWindow { background: #F4F6F8; }"
        "#sidebar { background: #0F2744; }"
        "#lblLogo { color: #FFFFFF; font-size: 22px; font-weight: 700; font-family: 'Segoe UI'; }"
        "#lblSousTitre, #lblModules { color: #A8C0D4; font-size: 12px; }"
        "#sidebar QPushButton { text-align: left; padding: 10px 14px; border: none; "
        "border-radius: 6px; color: #E8EEF4; background: transparent; font-size: 14px; }"
        "#sidebar QPushButton:hover { background: #1A3A5C; }"
        "#sidebar QPushButton:checked { background: #1ABC9C; color: #0F2744; font-weight: 600; }"
        "#lblTitreAccueil, #lblTitreFormateurs, #lblTitreCours { font-size: 22px; font-weight: 700; color: #0F2744; }"
        "#lblIntroAccueil { color: #5D6D7E; }"
        "QFrame#kpiFormateurs, QFrame#kpiCours, QFrame#kpiPlanifies, QFrame#kpiHeures {"
        " background: #FFFFFF; border: 1px solid #D5DDE5; border-radius: 8px; }"
        "#lblKpiFormateursValeur, #lblKpiCoursValeur, #lblKpiPlanifiesValeur, #lblKpiHeuresValeur {"
        " font-size: 28px; font-weight: 700; color: #0F2744; }"
        "QTabWidget::pane { border: 1px solid #D5DDE5; background: #FFFFFF; }"
        "QTabBar::tab { padding: 8px 16px; }"
        "QTableView { background: #FFFFFF; gridline-color: #E6ECF1; selection-background-color: #1ABC9C; "
        "selection-color: #0F2744; }"
        "QPushButton { padding: 7px 12px; border-radius: 4px; background: #0F2744; color: white; border: none; }"
        "QPushButton:hover { background: #1A3A5C; }"
        "#btnFormSupprimer, #btnCoursSupprimer { background: #C0392B; }"
        "#btnFormPdf, #btnFormPdfListe, #btnCoursPdf, #btnCoursCatalogue { background: #1ABC9C; color: #0F2744; }"
        "QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox { padding: 4px 6px; min-height: 24px; }"
        "QStatusBar { background: #FFFFFF; color: #5D6D7E; }"
    ));
}

void GCentreFormation::configurerControles()
{
    ui->leFormCin->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d{0,8}"), this));
    ui->leFormTel->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d{0,8}"), this));
    ui->deFormEmbauche->setDate(QDate::currentDate());
    ui->deCoursDebut->setDate(QDate::currentDate());
    ui->deCoursFin->setDate(QDate::currentDate().addDays(14));

    const QStringList specialites = {
        QString::fromUtf8("Informatique"),
        QString::fromUtf8("Reseaux"),
        QString::fromUtf8("Cybersecurite"),
        QString::fromUtf8("Gestion"),
        QString::fromUtf8("Langues"),
        QString::fromUtf8("Intelligence Artificielle"),
        QString::fromUtf8("Marketing"),
        QString::fromUtf8("Developpement Web")
    };
    ui->cbFormSpecialite->addItems(specialites);
    ui->cbFormStatut->addItems({QString::fromUtf8("Actif"), QString::fromUtf8("Inactif")});

    ui->cbFormRechSpec->addItem(QString::fromUtf8("Tous"));
    ui->cbFormRechSpec->addItems(specialites);
    ui->cbFormRechStatut->addItems({
        QString::fromUtf8("Tous"),
        QString::fromUtf8("Actif"),
        QString::fromUtf8("Inactif")
    });
    remplirComboTriFormateur(ui->cbFormTriCritere, false);
    remplirComboTriFormateur(ui->cbFormTriCritere2, true);
    remplirComboTriFormateur(ui->cbFormTriCritere3, true);
    remplirComboOrdreTri(ui->cbFormTriOrdre);
    remplirComboOrdreTri(ui->cbFormTriOrdre2);
    remplirComboOrdreTri(ui->cbFormTriOrdre3);

    ui->cbCoursNiveau->addItems({
        QString::fromUtf8("Debutant"),
        QString::fromUtf8("Intermediaire"),
        QString::fromUtf8("Avance")
    });
    ui->cbCoursStatut->addItems({
        QString::fromUtf8("Planifie"),
        QString::fromUtf8("En cours"),
        QString::fromUtf8("Termine"),
        QString::fromUtf8("Annule")
    });
    ui->cbCoursRechNiveau->addItem(QString::fromUtf8("Tous"));
    ui->cbCoursRechNiveau->addItems({
        QString::fromUtf8("Debutant"),
        QString::fromUtf8("Intermediaire"),
        QString::fromUtf8("Avance")
    });
    ui->cbCoursRechStatut->addItem(QString::fromUtf8("Tous"));
    ui->cbCoursRechStatut->addItems({
        QString::fromUtf8("Planifie"),
        QString::fromUtf8("En cours"),
        QString::fromUtf8("Termine"),
        QString::fromUtf8("Annule")
    });
    remplirComboTriCours(ui->cbCoursTriCritere, false);
    remplirComboTriCours(ui->cbCoursTriCritere2, true);
    remplirComboTriCours(ui->cbCoursTriCritere3, true);
    remplirComboOrdreTri(ui->cbCoursTriOrdre);
    remplirComboOrdreTri(ui->cbCoursTriOrdre2);
    remplirComboOrdreTri(ui->cbCoursTriOrdre3);

    const QList<QTableView*> tables = {
        ui->tableFormateurs, ui->tableFormRecherche, ui->tableFormCharge, ui->tableFormConflits,
        ui->tableCours, ui->tableCoursRecherche, ui->tableCoursAlertes
    };
    for (QTableView* t : tables) {
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->horizontalHeader()->setStretchLastSection(true);
        t->verticalHeader()->setVisible(false);
    }
}

void GCentreFormation::chargerTout()
{
    chargerTableFormateurs();
    chargerTableCours();
    chargerCombosFormateurs();
    appliquerModele(ui->tableFormRecherche, tmpFormateur.afficher());
    appliquerModele(ui->tableCoursRecherche, tmpCours.afficher());
    appliquerModele(ui->tableFormCharge, tmpFormateur.chargePedagogique());
    appliquerModele(ui->tableFormConflits, tmpFormateur.conflitsPlanning());
    appliquerModele(ui->tableCoursAlertes, tmpCours.alertes());
    rafraichirStatistiques();
    rafraichirAccueil();
}

void GCentreFormation::appliquerModele(QTableView* table, QSqlQueryModel* model)
{
    QAbstractItemModel* ancien = table->model();
    table->setModel(model);
    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);
    if (ancien && ancien != model) {
        delete ancien;
    }
}

void GCentreFormation::remplirComboDepuisModele(QComboBox* combo, QSqlQueryModel* model, bool avecTous)
{
    combo->clear();
    if (avecTous) {
        combo->addItem(QString::fromUtf8("Tous"), 0);
    }
    for (int i = 0; i < model->rowCount(); ++i) {
        combo->addItem(model->data(model->index(i, 1)).toString(),
                       model->data(model->index(i, 0)).toInt());
    }
    delete model;
}

void GCentreFormation::chargerCombosFormateurs()
{
    remplirComboDepuisModele(ui->cbCoursFormateur, tmpFormateur.listePourCombo(), false);
    remplirComboDepuisModele(ui->cbCoursRechFormateur, tmpFormateur.listePourCombo(), true);
    remplirComboDepuisModele(ui->cbAffFormateur, tmpFormateur.listePourCombo(), false);
    remplirComboDepuisModele(ui->cbAffCours, tmpCours.listePourCombo(), false);
}

void GCentreFormation::chargerTableFormateurs()
{
    appliquerModele(ui->tableFormateurs, tmpFormateur.afficher());
}

void GCentreFormation::chargerTableCours()
{
    appliquerModele(ui->tableCours, tmpCours.afficher());
}

void GCentreFormation::viderFormulaireFormateur()
{
    ui->leFormId->clear();
    ui->leFormCin->clear();
    ui->leFormNom->clear();
    ui->leFormPrenom->clear();
    ui->leFormEmail->clear();
    ui->leFormTel->clear();
    ui->cbFormSpecialite->setCurrentIndex(0);
    ui->deFormEmbauche->setDate(QDate::currentDate());
    ui->cbFormStatut->setCurrentIndex(0);
    ui->tableFormateurs->clearSelection();
}

void GCentreFormation::viderFormulaireCours()
{
    ui->leCoursId->clear();
    ui->leCoursTitre->clear();
    ui->cbCoursNiveau->setCurrentIndex(0);
    ui->sbCoursDuree->setValue(20);
    ui->dsCoursPrix->setValue(300);
    ui->deCoursDebut->setDate(QDate::currentDate());
    ui->deCoursFin->setDate(QDate::currentDate().addDays(14));
    ui->cbCoursStatut->setCurrentIndex(0);
    if (ui->cbCoursFormateur->count() > 0) {
        ui->cbCoursFormateur->setCurrentIndex(0);
    }
    ui->tableCours->clearSelection();
}

Formateur GCentreFormation::formateurDepuisFormulaire() const
{
    return Formateur(
        ui->leFormId->text().toInt(),
        ui->leFormCin->text(),
        ui->leFormNom->text(),
        ui->leFormPrenom->text(),
        ui->leFormEmail->text(),
        ui->leFormTel->text(),
        ui->cbFormSpecialite->currentText(),
        ui->deFormEmbauche->date(),
        ui->cbFormStatut->currentText());
}

Cours GCentreFormation::coursDepuisFormulaire() const
{
    return Cours(
        ui->leCoursId->text().toInt(),
        ui->leCoursTitre->text(),
        ui->cbCoursNiveau->currentText(),
        ui->sbCoursDuree->value(),
        ui->dsCoursPrix->value(),
        ui->deCoursDebut->date(),
        ui->deCoursFin->date(),
        ui->cbCoursStatut->currentText(),
        ui->cbCoursFormateur->currentData().toInt());
}

void GCentreFormation::informer(bool ok, const QString& succes, const QString& erreur)
{
    if (ok) {
        statusBar()->showMessage(succes, 5000);
    } else {
        statusBar()->showMessage(erreur, 8000);
        QMessageBox::warning(this, QString::fromUtf8("FormaPlus"), erreur);
    }
}

QString GCentreFormation::choisirFichierPdf(const QString& nomDefaut)
{
    return QFileDialog::getSaveFileName(
        this,
        QString::fromUtf8("Enregistrer le PDF"),
        nomDefaut,
        QString::fromUtf8("Documents PDF (*.pdf)"));
}

void GCentreFormation::rafraichirStatistiques()
{
    remplacerChart(ui->layoutChartFormSpec,
                   creerCamembert(QString::fromUtf8("Formateurs par spécialité"),
                                  tmpFormateur.statistiquesParSpecialite()));
    remplacerChart(ui->layoutChartFormStatut,
                   creerBarres(QString::fromUtf8("Formateurs par statut"),
                               tmpFormateur.statistiquesParStatut()));
    remplacerChart(ui->layoutChartCoursNiveau,
                   creerCamembert(QString::fromUtf8("Cours par niveau"),
                                  tmpCours.statistiquesParNiveau()));
    remplacerChart(ui->layoutChartCoursFormateur,
                   creerBarres(QString::fromUtf8("Cours par formateur"),
                               tmpCours.statistiquesParFormateur()));
    remplacerChart(ui->layoutChartCoursMois,
                   creerBarres(QString::fromUtf8("Cours par mois de début"),
                               tmpCours.statistiquesParMois()));
    remplacerChart(ui->layoutChartAccueilSpec,
                   creerCamembert(QString::fromUtf8("Formateurs par spécialité"),
                                  tmpFormateur.statistiquesParSpecialite()));
    remplacerChart(ui->layoutChartAccueilNiveau,
                   creerCamembert(QString::fromUtf8("Cours par niveau"),
                                  tmpCours.statistiquesParNiveau()));

    ui->lblFormStatsResume->setText(
        QString::fromUtf8("%1 formateurs · graphiques actualisés après chaque CRUD")
            .arg(tmpFormateur.compter()));
    ui->lblCoursStatsResume->setText(
        QString::fromUtf8("%1 cours · graphiques actualisés après chaque CRUD")
            .arg(tmpCours.compter()));
}

void GCentreFormation::rafraichirAccueil()
{
    ui->lblKpiFormateursValeur->setText(QString::number(tmpFormateur.compterActifs()));
    ui->lblKpiCoursValeur->setText(QString::number(tmpCours.compter()));
    ui->lblKpiPlanifiesValeur->setText(QString::number(tmpCours.compterParStatut(QString::fromUtf8("Planifie"))));
    ui->lblKpiHeuresValeur->setText(QString::number(tmpCours.sommeHeures(), 'f', 0));
}

void GCentreFormation::on_btnNavAccueil_clicked()
{
    ui->stackedPages->setCurrentIndex(0);
    rafraichirAccueil();
}

void GCentreFormation::on_btnNavFormateurs_clicked()
{
    ui->stackedPages->setCurrentIndex(1);
}

void GCentreFormation::on_btnNavCours_clicked()
{
    ui->stackedPages->setCurrentIndex(2);
}

void GCentreFormation::on_btnFormAjouter_clicked()
{
    Formateur f = formateurDepuisFormulaire();
    const bool ok = f.ajouter();
    informer(ok,
             QString::fromUtf8("Formateur ajouté."),
             f.getLastError());
    if (ok) {
        chargerTout();
        viderFormulaireFormateur();
    }
}

void GCentreFormation::on_btnFormModifier_clicked()
{
    Formateur f = formateurDepuisFormulaire();
    const bool ok = f.modifier();
    informer(ok,
             QString::fromUtf8("Formateur modifié."),
             f.getLastError());
    if (ok) {
        chargerTout();
    }
}

void GCentreFormation::on_btnFormSupprimer_clicked()
{
    const int id = ui->leFormId->text().toInt();
    if (id <= 0) {
        QMessageBox::warning(this, QString::fromUtf8("FormaPlus"),
                             QString::fromUtf8("Sélectionnez un formateur dans le tableau."));
        return;
    }
    if (QMessageBox::question(this, QString::fromUtf8("Confirmation"),
                              QString::fromUtf8("Supprimer le formateur n°%1 ?").arg(id))
            != QMessageBox::Yes) {
        return;
    }
    const bool ok = tmpFormateur.supprimer(id);
    informer(ok,
             QString::fromUtf8("Formateur supprimé."),
             tmpFormateur.getLastError());
    if (ok) {
        chargerTout();
        viderFormulaireFormateur();
    }
}

void GCentreFormation::on_btnFormVider_clicked()
{
    viderFormulaireFormateur();
}

void GCentreFormation::on_btnFormActualiser_clicked()
{
    chargerTout();
}

void GCentreFormation::on_btnFormPdf_clicked()
{
    const int id = ui->leFormId->text().toInt();
    if (id <= 0) {
        QMessageBox::warning(this, QString::fromUtf8("FormaPlus"),
                             QString::fromUtf8("Sélectionnez un formateur pour générer sa fiche."));
        return;
    }
    const QString chemin = choisirFichierPdf(QString("fiche_formateur_%1.pdf").arg(id));
    if (chemin.isEmpty()) {
        return;
    }
    const bool ok = tmpFormateur.exporterFichePdf(id, chemin);
    informer(ok,
             QString::fromUtf8("Fiche PDF enregistrée :\n%1").arg(chemin),
             tmpFormateur.getLastError());
}

void GCentreFormation::on_tableFormateurs_clicked(const QModelIndex& index)
{
    if (!index.isValid() || !ui->tableFormateurs->model()) {
        return;
    }
    const QAbstractItemModel* m = ui->tableFormateurs->model();
    const int row = index.row();
    ui->leFormId->setText(m->index(row, 0).data().toString());
    ui->leFormCin->setText(m->index(row, 1).data().toString());
    ui->leFormNom->setText(m->index(row, 2).data().toString());
    ui->leFormPrenom->setText(m->index(row, 3).data().toString());
    ui->leFormEmail->setText(m->index(row, 4).data().toString());
    ui->leFormTel->setText(m->index(row, 5).data().toString());
    ui->cbFormSpecialite->setCurrentText(m->index(row, 6).data().toString());
    ui->deFormEmbauche->setDate(QDate::fromString(m->index(row, 7).data().toString(), "dd/MM/yyyy"));
    ui->cbFormStatut->setCurrentText(m->index(row, 8).data().toString());
}

void GCentreFormation::on_btnFormRechercher_clicked()
{
    appliquerModele(ui->tableFormRecherche,
                    tmpFormateur.rechercher(ui->leFormRechNom->text(),
                                            ui->cbFormRechSpec->currentText(),
                                            ui->cbFormRechStatut->currentText()));
    statusBar()->showMessage(QString::fromUtf8("Recherche formateurs effectuée."), 3000);
}

void GCentreFormation::on_btnFormTrier_clicked()
{
    QStringList criteres;
    QStringList ordres;
    collecterTri(ui->cbFormTriCritere, ui->cbFormTriOrdre, criteres, ordres);
    collecterTri(ui->cbFormTriCritere2, ui->cbFormTriOrdre2, criteres, ordres);
    collecterTri(ui->cbFormTriCritere3, ui->cbFormTriOrdre3, criteres, ordres);
    appliquerModele(ui->tableFormRecherche, tmpFormateur.trierMulti(criteres, ordres));
    statusBar()->showMessage(
        QString::fromUtf8("Tri multicritères formateurs (%1 critère(s)).").arg(criteres.size()), 3000);
}

void GCentreFormation::on_btnFormPdfListe_clicked()
{
    const QString chemin = choisirFichierPdf(QStringLiteral("liste_formateurs.pdf"));
    if (chemin.isEmpty()) {
        return;
    }
    const bool ok = tmpFormateur.exporterListePdf(ui->leFormRechNom->text(),
                                                  ui->cbFormRechSpec->currentText(),
                                                  ui->cbFormRechStatut->currentText(),
                                                  chemin);
    informer(ok,
             QString::fromUtf8("Liste PDF enregistrée :\n%1").arg(chemin),
             tmpFormateur.getLastError());
}

void GCentreFormation::on_btnFormRefreshMetiers_clicked()
{
    appliquerModele(ui->tableFormCharge, tmpFormateur.chargePedagogique());
    appliquerModele(ui->tableFormConflits, tmpFormateur.conflitsPlanning());
    statusBar()->showMessage(QString::fromUtf8("Charge et conflits actualisés."), 3000);
}

void GCentreFormation::on_btnCoursAjouter_clicked()
{
    Cours c = coursDepuisFormulaire();
    const bool ok = c.ajouter();
    informer(ok,
             QString::fromUtf8("Cours ajouté."),
             c.getLastError());
    if (ok) {
        chargerTout();
        viderFormulaireCours();
    }
}

void GCentreFormation::on_btnCoursModifier_clicked()
{
    Cours c = coursDepuisFormulaire();
    const bool ok = c.modifier();
    informer(ok,
             QString::fromUtf8("Cours modifié."),
             c.getLastError());
    if (ok) {
        chargerTout();
    }
}

void GCentreFormation::on_btnCoursSupprimer_clicked()
{
    const int id = ui->leCoursId->text().toInt();
    if (id <= 0) {
        QMessageBox::warning(this, QString::fromUtf8("FormaPlus"),
                             QString::fromUtf8("Sélectionnez un cours dans le tableau."));
        return;
    }
    if (QMessageBox::question(this, QString::fromUtf8("Confirmation"),
                              QString::fromUtf8("Supprimer le cours n°%1 ?").arg(id))
            != QMessageBox::Yes) {
        return;
    }
    const bool ok = tmpCours.supprimer(id);
    informer(ok,
             QString::fromUtf8("Cours supprimé."),
             tmpCours.getLastError());
    if (ok) {
        chargerTout();
        viderFormulaireCours();
    }
}

void GCentreFormation::on_btnCoursVider_clicked()
{
    viderFormulaireCours();
}

void GCentreFormation::on_btnCoursActualiser_clicked()
{
    chargerTout();
}

void GCentreFormation::on_btnCoursPdf_clicked()
{
    const int id = ui->leCoursId->text().toInt();
    if (id <= 0) {
        QMessageBox::warning(this, QString::fromUtf8("FormaPlus"),
                             QString::fromUtf8("Sélectionnez un cours pour générer sa fiche."));
        return;
    }
    const QString chemin = choisirFichierPdf(QString("fiche_cours_%1.pdf").arg(id));
    if (chemin.isEmpty()) {
        return;
    }
    const bool ok = tmpCours.exporterFichePdf(id, chemin);
    informer(ok,
             QString::fromUtf8("Fiche PDF enregistrée :\n%1").arg(chemin),
             tmpCours.getLastError());
}

void GCentreFormation::on_btnCoursCatalogue_clicked()
{
    const QString chemin = choisirFichierPdf(QStringLiteral("catalogue_cours.pdf"));
    if (chemin.isEmpty()) {
        return;
    }
    const bool ok = tmpCours.exporterCataloguePdf(chemin);
    informer(ok,
             QString::fromUtf8("Catalogue PDF enregistré :\n%1").arg(chemin),
             tmpCours.getLastError());
}

void GCentreFormation::on_tableCours_clicked(const QModelIndex& index)
{
    if (!index.isValid() || !ui->tableCours->model()) {
        return;
    }
    const QAbstractItemModel* m = ui->tableCours->model();
    const int row = index.row();
    ui->leCoursId->setText(m->index(row, 0).data().toString());
    ui->leCoursTitre->setText(m->index(row, 1).data().toString());
    ui->cbCoursNiveau->setCurrentText(m->index(row, 2).data().toString());
    ui->sbCoursDuree->setValue(m->index(row, 3).data().toInt());
    ui->dsCoursPrix->setValue(m->index(row, 4).data().toDouble());
    ui->deCoursDebut->setDate(QDate::fromString(m->index(row, 5).data().toString(), "dd/MM/yyyy"));
    ui->deCoursFin->setDate(QDate::fromString(m->index(row, 6).data().toString(), "dd/MM/yyyy"));
    ui->cbCoursStatut->setCurrentText(m->index(row, 7).data().toString());
    const int idForm = m->index(row, 8).data().toInt();
    const int pos = ui->cbCoursFormateur->findData(idForm);
    if (pos >= 0) {
        ui->cbCoursFormateur->setCurrentIndex(pos);
    }
}

void GCentreFormation::on_btnCoursRechercher_clicked()
{
    appliquerModele(ui->tableCoursRecherche,
                    tmpCours.rechercher(ui->leCoursRechTitre->text(),
                                        ui->cbCoursRechNiveau->currentText(),
                                        ui->cbCoursRechStatut->currentText(),
                                        ui->cbCoursRechFormateur->currentData().toInt()));
    statusBar()->showMessage(QString::fromUtf8("Recherche cours effectuée."), 3000);
}

void GCentreFormation::on_btnCoursTrier_clicked()
{
    QStringList criteres;
    QStringList ordres;
    collecterTri(ui->cbCoursTriCritere, ui->cbCoursTriOrdre, criteres, ordres);
    collecterTri(ui->cbCoursTriCritere2, ui->cbCoursTriOrdre2, criteres, ordres);
    collecterTri(ui->cbCoursTriCritere3, ui->cbCoursTriOrdre3, criteres, ordres);
    appliquerModele(ui->tableCoursRecherche, tmpCours.trierMulti(criteres, ordres));
    statusBar()->showMessage(
        QString::fromUtf8("Tri multicritères cours (%1 critère(s)).").arg(criteres.size()), 3000);
}

void GCentreFormation::on_btnCoursAffecter_clicked()
{
    const int idCours = ui->cbAffCours->currentData().toInt();
    const int idForm = ui->cbAffFormateur->currentData().toInt();
    const bool ok = tmpCours.affecterFormateur(idCours, idForm);
    informer(ok,
             QString::fromUtf8("Formateur affecté au cours."),
             tmpCours.getLastError());
    if (ok) {
        chargerTout();
    }
}

void GCentreFormation::on_btnCoursRefreshAlertes_clicked()
{
    appliquerModele(ui->tableCoursAlertes, tmpCours.alertes());
    statusBar()->showMessage(QString::fromUtf8("Alertes actualisées."), 3000);
}
