#include "cours.h"

#include <utility>
#include <QSqlError>
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QPageSize>
#include <QPageLayout>
#include <QDateTime>

static const char* SQL_COURS_SELECT =
    "SELECT c.ID_COURS, c.TITRE, c.NIVEAU, c.DUREE_H, c.PRIX, "
    "TO_CHAR(c.DATE_DEBUT,'DD/MM/YYYY') AS DATE_DEBUT, "
    "TO_CHAR(c.DATE_FIN,'DD/MM/YYYY') AS DATE_FIN, c.STATUT, "
    "c.ID_FORMATEUR, f.NOM || ' ' || f.PRENOM AS FORMATEUR "
    "FROM COURS c JOIN FORMATEUR f ON f.ID_FORMATEUR = c.ID_FORMATEUR ";

Cours::Cours()
    : id(0), dureeH(0), prix(0.0), idFormateur(0)
{
}

Cours::Cours(int id, const QString& titre, const QString& niveau, int dureeH, double prix,
             const QDate& dateDebut, const QDate& dateFin, const QString& statut, int idFormateur)
    : id(id), titre(titre.trimmed()), niveau(niveau.trimmed()), dureeH(dureeH), prix(prix),
      dateDebut(dateDebut), dateFin(dateFin), statut(statut.trimmed()), idFormateur(idFormateur)
{
}

bool Cours::valider(QString& message) const
{
    if (titre.length() < 3) {
        message = QString::fromUtf8("Le titre doit contenir au moins 3 caractères.");
        return false;
    }
    const QStringList niveaux = {
        QString::fromUtf8("Debutant"),
        QString::fromUtf8("Intermediaire"),
        QString::fromUtf8("Avance")
    };
    if (!niveaux.contains(niveau)) {
        message = QString::fromUtf8("Niveau invalide.");
        return false;
    }
    if (dureeH <= 0) {
        message = QString::fromUtf8("La durée doit être strictement positive.");
        return false;
    }
    if (prix <= 0.0) {
        message = QString::fromUtf8("Le prix doit être strictement positif.");
        return false;
    }
    if (!dateDebut.isValid() || !dateFin.isValid()) {
        message = QString::fromUtf8("Les dates du cours sont invalides.");
        return false;
    }
    if (dateFin < dateDebut) {
        message = QString::fromUtf8("La date de fin doit être postérieure ou égale à la date de début.");
        return false;
    }
    const QStringList statuts = {
        QString::fromUtf8("Planifie"),
        QString::fromUtf8("En cours"),
        QString::fromUtf8("Termine"),
        QString::fromUtf8("Annule")
    };
    if (!statuts.contains(statut)) {
        message = QString::fromUtf8("Statut de cours invalide.");
        return false;
    }
    if (idFormateur <= 0) {
        message = QString::fromUtf8("Un formateur doit être affecté au cours.");
        return false;
    }
    return true;
}

void Cours::setOracleError(const QSqlQuery& query)
{
    const QString oracle = query.lastError().text();
    if (oracle.contains(QLatin1String("ORA-02291"))) {
        lastError = QString::fromUtf8("Le formateur choisi n'existe pas.");
    } else if (oracle.contains(QLatin1String("ORA-02290"))) {
        lastError = QString::fromUtf8("Une contrainte de la base de données a été violée (dates, prix ou statut).");
    } else {
        lastError = oracle;
    }
}

QSqlQueryModel* Cours::executerSelect(QSqlQuery& query)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    if (!query.exec()) {
        setOracleError(query);
    }
    model->setQuery(std::move(query));
    model->setHeaderData(0, Qt::Horizontal, QString::fromUtf8("ID"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromUtf8("Titre"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromUtf8("Niveau"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromUtf8("Durée (h)"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromUtf8("Prix"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromUtf8("Début"));
    model->setHeaderData(6, Qt::Horizontal, QString::fromUtf8("Fin"));
    model->setHeaderData(7, Qt::Horizontal, QString::fromUtf8("Statut"));
    model->setHeaderData(8, Qt::Horizontal, QString::fromUtf8("ID formateur"));
    model->setHeaderData(9, Qt::Horizontal, QString::fromUtf8("Formateur"));
    return model;
}

bool Cours::ajouter()
{
    QString message;
    if (!valider(message)) {
        lastError = message;
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO COURS (TITRE, NIVEAU, DUREE_H, PRIX, DATE_DEBUT, DATE_FIN, STATUT, ID_FORMATEUR) "
        "VALUES (:titre, :niveau, :duree, :prix, TO_DATE(:debut,'YYYY-MM-DD'), "
        "TO_DATE(:fin,'YYYY-MM-DD'), :statut, :id_formateur)");
    query.bindValue(":titre", titre);
    query.bindValue(":niveau", niveau);
    query.bindValue(":duree", dureeH);
    query.bindValue(":prix", prix);
    query.bindValue(":debut", dateDebut.toString(QLatin1String("yyyy-MM-dd")));
    query.bindValue(":fin", dateFin.toString(QLatin1String("yyyy-MM-dd")));
    query.bindValue(":statut", statut);
    query.bindValue(":id_formateur", idFormateur);

    if (!query.exec()) {
        setOracleError(query);
        return false;
    }
    lastError.clear();
    return true;
}

bool Cours::modifier()
{
    QString message;
    if (id <= 0) {
        lastError = QString::fromUtf8("Sélectionnez un cours à modifier.");
        return false;
    }
    if (!valider(message)) {
        lastError = message;
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "UPDATE COURS SET TITRE = :titre, NIVEAU = :niveau, DUREE_H = :duree, PRIX = :prix, "
        "DATE_DEBUT = TO_DATE(:debut,'YYYY-MM-DD'), DATE_FIN = TO_DATE(:fin,'YYYY-MM-DD'), "
        "STATUT = :statut, ID_FORMATEUR = :id_formateur "
        "WHERE ID_COURS = :id");
    query.bindValue(":titre", titre);
    query.bindValue(":niveau", niveau);
    query.bindValue(":duree", dureeH);
    query.bindValue(":prix", prix);
    query.bindValue(":debut", dateDebut.toString(QLatin1String("yyyy-MM-dd")));
    query.bindValue(":fin", dateFin.toString(QLatin1String("yyyy-MM-dd")));
    query.bindValue(":statut", statut);
    query.bindValue(":id_formateur", idFormateur);
    query.bindValue(":id", id);

    if (!query.exec()) {
        setOracleError(query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        lastError = QString::fromUtf8("Aucun cours mis à jour.");
        return false;
    }
    lastError.clear();
    return true;
}

bool Cours::supprimer(int idCours)
{
    if (idCours <= 0) {
        lastError = QString::fromUtf8("Sélectionnez un cours à supprimer.");
        return false;
    }
    QSqlQuery query;
    query.prepare("DELETE FROM COURS WHERE ID_COURS = :id");
    query.bindValue(":id", idCours);
    if (!query.exec()) {
        setOracleError(query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        lastError = QString::fromUtf8("Cours introuvable.");
        return false;
    }
    lastError.clear();
    return true;
}

bool Cours::affecterFormateur(int idCours, int idFormateurCible)
{
    if (idCours <= 0 || idFormateurCible <= 0) {
        lastError = QString::fromUtf8("Choisissez un cours et un formateur.");
        return false;
    }
    QSqlQuery query;
    query.prepare("UPDATE COURS SET ID_FORMATEUR = :id_formateur WHERE ID_COURS = :id");
    query.bindValue(":id_formateur", idFormateurCible);
    query.bindValue(":id", idCours);
    if (!query.exec()) {
        setOracleError(query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        lastError = QString::fromUtf8("Cours introuvable.");
        return false;
    }
    lastError.clear();
    return true;
}

QSqlQueryModel* Cours::afficher()
{
    QSqlQuery query;
    query.prepare(QString(SQL_COURS_SELECT) + "ORDER BY c.ID_COURS");
    return executerSelect(query);
}

QSqlQueryModel* Cours::rechercher(const QString& titreCritere,
                                 const QString& niveauCritere,
                                 const QString& statutCritere,
                                 int idFormateurCritere)
{
    QString sql = QString(SQL_COURS_SELECT) + "WHERE 1=1 ";
    if (!titreCritere.trimmed().isEmpty()) {
        sql += "AND UPPER(c.TITRE) LIKE UPPER(:titre) ";
    }
    if (!niveauCritere.trimmed().isEmpty() && niveauCritere != QLatin1String("Tous")) {
        sql += "AND c.NIVEAU = :niveau ";
    }
    if (!statutCritere.trimmed().isEmpty() && statutCritere != QLatin1String("Tous")) {
        sql += "AND c.STATUT = :statut ";
    }
    if (idFormateurCritere > 0) {
        sql += "AND c.ID_FORMATEUR = :id_formateur ";
    }
    sql += "ORDER BY c.TITRE";

    QSqlQuery query;
    query.prepare(sql);
    if (!titreCritere.trimmed().isEmpty()) {
        query.bindValue(":titre", QString("%%1%").arg(titreCritere.trimmed()));
    }
    if (!niveauCritere.trimmed().isEmpty() && niveauCritere != QLatin1String("Tous")) {
        query.bindValue(":niveau", niveauCritere.trimmed());
    }
    if (!statutCritere.trimmed().isEmpty() && statutCritere != QLatin1String("Tous")) {
        query.bindValue(":statut", statutCritere.trimmed());
    }
    if (idFormateurCritere > 0) {
        query.bindValue(":id_formateur", idFormateurCritere);
    }
    return executerSelect(query);
}

QSqlQueryModel* Cours::trierMulti(const QStringList& criteres, const QStringList& ordres)
{
    const QStringList colonnes = {
        QLatin1String("ID_COURS"), QLatin1String("TITRE"), QLatin1String("NIVEAU"),
        QLatin1String("DUREE_H"), QLatin1String("PRIX"), QLatin1String("DATE_DEBUT"),
        QLatin1String("STATUT")
    };
    QStringList orderParts;
    const int n = qMin(criteres.size(), ordres.size());
    for (int i = 0; i < n; ++i) {
        QString col = criteres.at(i).trimmed().toUpper();
        if (col.isEmpty()) {
            continue;
        }
        if (!colonnes.contains(col)) {
            continue;
        }
        const QString sens = (ordres.at(i).toUpper() == QLatin1String("DESC"))
                ? QLatin1String("DESC") : QLatin1String("ASC");
        orderParts << QString("c.%1 %2").arg(col, sens);
    }
    if (orderParts.isEmpty()) {
        orderParts << QStringLiteral("c.TITRE ASC");
    }

    QSqlQuery query;
    query.prepare(QString(SQL_COURS_SELECT) + QString("ORDER BY %1").arg(orderParts.join(", ")));
    return executerSelect(query);
}

QSqlQueryModel* Cours::trier(const QString& critere, const QString& ordre)
{
    return trierMulti(QStringList{critere}, QStringList{ordre});
}

QSqlQueryModel* Cours::listePourCombo()
{
    QSqlQuery query;
    query.prepare("SELECT ID_COURS, TITRE || ' (' || STATUT || ')' AS LIBELLE FROM COURS ORDER BY TITRE");
    QSqlQueryModel* model = new QSqlQueryModel();
    if (!query.exec()) {
        setOracleError(query);
    }
    model->setQuery(std::move(query));
    return model;
}

QMap<QString, int> Cours::statistiquesParNiveau()
{
    QMap<QString, int> stats;
    QSqlQuery query;
    query.prepare("SELECT NIVEAU, COUNT(*) FROM COURS GROUP BY NIVEAU ORDER BY NIVEAU");
    if (!query.exec()) {
        setOracleError(query);
        return stats;
    }
    while (query.next()) {
        stats.insert(query.value(0).toString(), query.value(1).toInt());
    }
    return stats;
}

QMap<QString, int> Cours::statistiquesParFormateur()
{
    QMap<QString, int> stats;
    QSqlQuery query;
    query.prepare(
        "SELECT f.NOM || ' ' || f.PRENOM, COUNT(*) "
        "FROM COURS c JOIN FORMATEUR f ON f.ID_FORMATEUR = c.ID_FORMATEUR "
        "GROUP BY f.NOM, f.PRENOM ORDER BY COUNT(*) DESC");
    if (!query.exec()) {
        setOracleError(query);
        return stats;
    }
    while (query.next()) {
        stats.insert(query.value(0).toString(), query.value(1).toInt());
    }
    return stats;
}

QMap<QString, int> Cours::statistiquesParMois()
{
    QMap<QString, int> stats;
    QSqlQuery query;
    query.prepare(
        "SELECT TO_CHAR(DATE_DEBUT,'YYYY-MM') AS MOIS, COUNT(*) "
        "FROM COURS GROUP BY TO_CHAR(DATE_DEBUT,'YYYY-MM') ORDER BY MOIS");
    if (!query.exec()) {
        setOracleError(query);
        return stats;
    }
    while (query.next()) {
        stats.insert(query.value(0).toString(), query.value(1).toInt());
    }
    return stats;
}

QSqlQueryModel* Cours::alertes()
{
    QSqlQuery query;
    query.prepare(
        "SELECT c.ID_COURS, c.TITRE, TO_CHAR(c.DATE_DEBUT,'DD/MM/YYYY') AS DEBUT, "
        "c.STATUT, f.NOM || ' ' || f.PRENOM AS FORMATEUR, "
        "'Debut imminent (7 jours)' AS ALERTE "
        "FROM COURS c JOIN FORMATEUR f ON f.ID_FORMATEUR = c.ID_FORMATEUR "
        "WHERE c.STATUT = 'Planifie' AND c.DATE_DEBUT BETWEEN TRUNC(SYSDATE) AND TRUNC(SYSDATE)+7 "
        "UNION ALL "
        "SELECT c.ID_COURS, c.TITRE, TO_CHAR(c.DATE_DEBUT,'DD/MM/YYYY'), "
        "c.STATUT, f.NOM || ' ' || f.PRENOM, "
        "'Formateur inactif' "
        "FROM COURS c JOIN FORMATEUR f ON f.ID_FORMATEUR = c.ID_FORMATEUR "
        "WHERE f.STATUT = 'Inactif' AND c.STATUT IN ('Planifie','En cours') "
        "UNION ALL "
        "SELECT a.ID_COURS, a.TITRE, TO_CHAR(a.DATE_DEBUT,'DD/MM/YYYY'), "
        "a.STATUT, f.NOM || ' ' || f.PRENOM, "
        "'Chevauchement avec le cours ' || b.ID_COURS "
        "FROM COURS a "
        "JOIN COURS b ON a.ID_FORMATEUR = b.ID_FORMATEUR AND a.ID_COURS < b.ID_COURS "
        "JOIN FORMATEUR f ON f.ID_FORMATEUR = a.ID_FORMATEUR "
        "WHERE a.STATUT <> 'Annule' AND b.STATUT <> 'Annule' "
        "AND a.DATE_DEBUT <= b.DATE_FIN AND b.DATE_DEBUT <= a.DATE_FIN "
        "ORDER BY 1");
    QSqlQueryModel* model = new QSqlQueryModel();
    if (!query.exec()) {
        setOracleError(query);
    }
    model->setQuery(std::move(query));
    model->setHeaderData(0, Qt::Horizontal, QString::fromUtf8("ID"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromUtf8("Titre"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromUtf8("Début"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromUtf8("Statut"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromUtf8("Formateur"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromUtf8("Alerte"));
    return model;
}

int Cours::compter()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM COURS");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int Cours::compterParStatut(const QString& statutCible)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM COURS WHERE STATUT = :statut");
    query.bindValue(":statut", statutCible);
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

double Cours::sommeHeures()
{
    QSqlQuery query;
    query.prepare("SELECT NVL(SUM(DUREE_H),0) FROM COURS WHERE STATUT <> 'Annule'");
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

QString Cours::htmlEntete(const QString& titreDoc) const
{
    return QString::fromUtf8(
        "<div style='background-color:#0F2744;color:white;padding:18px 24px;'>"
        "<h1 style='margin:0;font-family:Arial;'>FormaPlus</h1>"
        "<p style='margin:4px 0 0 0;'>Centre de formation — GCentreFormation</p>"
        "</div>"
        "<h2 style='color:#0F2744;font-family:Arial;margin-top:20px;'>%1</h2>"
        "<p style='color:#555;font-family:Arial;'>Document généré le %2</p>")
        .arg(titreDoc, QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm"));
}

QString Cours::htmlPied() const
{
    return QString::fromUtf8(
        "<hr/><p style='font-size:10px;color:#777;font-family:Arial;'>"
        "Catalogue / fiche officielle FormaPlus. Document personnalisé — pas une capture d'écran.</p>");
}

bool Cours::ecrirePdf(const QString& html, const QString& chemin)
{
    QPdfWriter writer(chemin);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    writer.setTitle(QString::fromUtf8("FormaPlus — Cours"));

    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(QSizeF(writer.width(), writer.height()));
    QPainter painter(&writer);
    if (!painter.isActive()) {
        lastError = QString::fromUtf8("Impossible d'écrire le fichier PDF.");
        return false;
    }
    doc.drawContents(&painter);
    lastError.clear();
    return true;
}

bool Cours::exporterFichePdf(int idCours, const QString& chemin)
{
    QSqlQuery query;
    query.prepare(
        "SELECT c.ID_COURS, c.TITRE, c.NIVEAU, c.DUREE_H, c.PRIX, "
        "TO_CHAR(c.DATE_DEBUT,'DD/MM/YYYY'), TO_CHAR(c.DATE_FIN,'DD/MM/YYYY'), "
        "c.STATUT, f.NOM, f.PRENOM, f.SPECIALITE, f.EMAIL "
        "FROM COURS c JOIN FORMATEUR f ON f.ID_FORMATEUR = c.ID_FORMATEUR "
        "WHERE c.ID_COURS = :id");
    query.bindValue(":id", idCours);
    if (!query.exec() || !query.next()) {
        lastError = QString::fromUtf8("Cours introuvable pour l'export PDF.");
        return false;
    }

    const QString html = htmlEntete(QString::fromUtf8("Fiche cours"))
        + QString::fromUtf8(
            "<table width='100%' cellpadding='6' style='font-family:Arial;'>"
            "<tr><td><b>Référence</b></td><td>CRS-%1</td><td><b>Statut</b></td><td>%2</td></tr>"
            "<tr><td><b>Titre</b></td><td colspan='3'>%3</td></tr>"
            "<tr><td><b>Niveau</b></td><td>%4</td><td><b>Durée</b></td><td>%5 heures</td></tr>"
            "<tr><td><b>Prix</b></td><td>%6 DT</td><td><b>Période</b></td><td>%7 — %8</td></tr>"
            "</table>"
            "<h3 style='color:#0F2744;font-family:Arial;'>Formateur responsable</h3>"
            "<p style='font-family:Arial;'>%9 %10<br/>Spécialité : %11<br/>Contact : %12</p>")
            .arg(query.value(0).toString(), query.value(7).toString(),
                 query.value(1).toString(), query.value(2).toString(),
                 query.value(3).toString(),
                 QString::number(query.value(4).toDouble(), 'f', 2),
                 query.value(5).toString(), query.value(6).toString(),
                 query.value(8).toString(), query.value(9).toString(),
                 query.value(10).toString(), query.value(11).toString())
        + htmlPied();

    return ecrirePdf(html, chemin);
}

bool Cours::exporterCataloguePdf(const QString& chemin)
{
    QSqlQuery query;
    query.prepare(
        "SELECT c.ID_COURS, c.TITRE, c.NIVEAU, c.DUREE_H, c.PRIX, "
        "TO_CHAR(c.DATE_DEBUT,'DD/MM/YYYY'), c.STATUT, f.NOM || ' ' || f.PRENOM "
        "FROM COURS c JOIN FORMATEUR f ON f.ID_FORMATEUR = c.ID_FORMATEUR "
        "WHERE c.STATUT <> 'Annule' ORDER BY c.DATE_DEBUT, c.TITRE");
    if (!query.exec()) {
        setOracleError(query);
        return false;
    }

    QString lignes;
    while (query.next()) {
        lignes += QString::fromUtf8(
            "<tr><td>CRS-%1</td><td>%2</td><td>%3</td><td align='right'>%4</td>"
            "<td align='right'>%5</td><td>%6</td><td>%7</td><td>%8</td></tr>")
            .arg(query.value(0).toString(), query.value(1).toString(),
                 query.value(2).toString(), query.value(3).toString(),
                 QString::number(query.value(4).toDouble(), 'f', 2),
                 query.value(5).toString(), query.value(6).toString(),
                 query.value(7).toString());
    }
    if (lignes.isEmpty()) {
        lignes = QString::fromUtf8("<tr><td colspan='8'>Aucun cours au catalogue.</td></tr>");
    }

    const QString html = htmlEntete(QString::fromUtf8("Catalogue des cours"))
        + QString::fromUtf8(
            "<table width='100%' border='1' cellspacing='0' cellpadding='5' "
            "style='border-collapse:collapse;font-family:Arial;font-size:11px;'>"
            "<tr style='background:#0F2744;color:white;'>"
            "<th>Réf.</th><th>Titre</th><th>Niveau</th><th>Heures</th>"
            "<th>Prix (DT)</th><th>Début</th><th>Statut</th><th>Formateur</th></tr>"
            "%1</table>").arg(lignes)
        + htmlPied();

    return ecrirePdf(html, chemin);
}
