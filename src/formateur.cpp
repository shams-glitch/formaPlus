#include "formateur.h"
#include "session.h"

#include <utility>
#include <QSqlError>
#include <QRegularExpression>
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QPageSize>
#include <QPageLayout>
#include <QDateTime>

static const char* SQL_FORMATEUR_SELECT =
    "SELECT f.ID_FORMATEUR, f.CIN, f.NOM, f.PRENOM, f.EMAIL, f.TELEPHONE, "
    "f.SPECIALITE, TO_CHAR(f.DATE_EMBAUCHE,'DD/MM/YYYY') AS DATE_EMBAUCHE, f.STATUT "
    "FROM FORMATEUR f ";

Formateur::Formateur()
    : id(0)
{
}

Formateur::Formateur(int id, const QString& cin, const QString& nom, const QString& prenom,
                     const QString& email, const QString& telephone, const QString& specialite,
                     const QDate& dateEmbauche, const QString& statut)
    : id(id), cin(cin.trimmed()), nom(nom.trimmed()), prenom(prenom.trimmed()),
      email(email.trimmed()), telephone(telephone.trimmed()), specialite(specialite.trimmed()),
      dateEmbauche(dateEmbauche), statut(statut.trimmed())
{
}

bool Formateur::valider(QString& message) const
{
    if (!QRegularExpression("^\\d{8}$").match(cin).hasMatch()) {
        message = QString::fromUtf8("Le CIN doit contenir exactement 8 chiffres.");
        return false;
    }
    if (!QRegularExpression("^[A-Za-zÀ-ÿ '\\-]{2,50}$").match(nom).hasMatch()) {
        message = QString::fromUtf8("Le nom est invalide (2 à 50 lettres).");
        return false;
    }
    if (!QRegularExpression("^[A-Za-zÀ-ÿ '\\-]{2,50}$").match(prenom).hasMatch()) {
        message = QString::fromUtf8("Le prénom est invalide (2 à 50 lettres).");
        return false;
    }
    if (!QRegularExpression("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$").match(email).hasMatch()) {
        message = QString::fromUtf8("L'adresse e-mail est invalide.");
        return false;
    }
    if (!QRegularExpression("^\\d{8}$").match(telephone).hasMatch()) {
        message = QString::fromUtf8("Le téléphone doit contenir exactement 8 chiffres.");
        return false;
    }
    if (specialite.isEmpty()) {
        message = QString::fromUtf8("La spécialité est obligatoire.");
        return false;
    }
    if (!dateEmbauche.isValid()) {
        message = QString::fromUtf8("La date d'embauche est invalide.");
        return false;
    }
    if (statut != QLatin1String("Actif") && statut != QLatin1String("Inactif")) {
        message = QString::fromUtf8("Le statut doit être Actif ou Inactif.");
        return false;
    }
    return true;
}

void Formateur::setOracleError(const QSqlQuery& query)
{
    const QString oracle = query.lastError().text();
    if (oracle.contains(QLatin1String("ORA-00001"))) {
        lastError = QString::fromUtf8("CIN ou e-mail déjà utilisé.");
    } else if (oracle.contains(QLatin1String("ORA-02292"))) {
        lastError = QString::fromUtf8("Suppression impossible : des cours sont encore rattachés à ce formateur.");
    } else if (oracle.contains(QLatin1String("ORA-02290"))) {
        lastError = QString::fromUtf8("Une contrainte de la base de données a été violée.");
    } else {
        lastError = oracle;
    }
}

QSqlQueryModel* Formateur::executerSelect(QSqlQuery& query)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    if (!query.exec()) {
        setOracleError(query);
        model->setQuery(std::move(query));
        return model;
    }
    model->setQuery(std::move(query));
    model->setHeaderData(0, Qt::Horizontal, QString::fromUtf8("ID"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromUtf8("CIN"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromUtf8("Nom"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromUtf8("Prénom"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromUtf8("E-mail"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromUtf8("Téléphone"));
    model->setHeaderData(6, Qt::Horizontal, QString::fromUtf8("Spécialité"));
    model->setHeaderData(7, Qt::Horizontal, QString::fromUtf8("Embauche"));
    model->setHeaderData(8, Qt::Horizontal, QString::fromUtf8("Statut"));
    return model;
}

bool Formateur::ajouter()
{
    if (!Session::exigerAdmin(&lastError)) {
        return false;
    }
    QString message;
    if (!valider(message)) {
        lastError = message;
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO FORMATEUR (CIN, NOM, PRENOM, EMAIL, TELEPHONE, SPECIALITE, DATE_EMBAUCHE, STATUT) "
        "VALUES (:cin, :nom, :prenom, :email, :telephone, :specialite, TO_DATE(:date_embauche,'YYYY-MM-DD'), :statut)");
    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":specialite", specialite);
    query.bindValue(":date_embauche", dateEmbauche.toString(QLatin1String("yyyy-MM-dd")));
    query.bindValue(":statut", statut);

    if (!query.exec()) {
        setOracleError(query);
        return false;
    }
    lastError.clear();
    return true;
}

bool Formateur::modifier()
{
    if (!Session::exigerAdmin(&lastError)) {
        return false;
    }
    QString message;
    if (id <= 0) {
        lastError = QString::fromUtf8("Sélectionnez un formateur à modifier.");
        return false;
    }
    if (!valider(message)) {
        lastError = message;
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "UPDATE FORMATEUR SET CIN = :cin, NOM = :nom, PRENOM = :prenom, EMAIL = :email, "
        "TELEPHONE = :telephone, SPECIALITE = :specialite, "
        "DATE_EMBAUCHE = TO_DATE(:date_embauche,'YYYY-MM-DD'), STATUT = :statut "
        "WHERE ID_FORMATEUR = :id");
    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":specialite", specialite);
    query.bindValue(":date_embauche", dateEmbauche.toString(QLatin1String("yyyy-MM-dd")));
    query.bindValue(":statut", statut);
    query.bindValue(":id", id);

    if (!query.exec()) {
        setOracleError(query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        lastError = QString::fromUtf8("Aucun formateur mis à jour.");
        return false;
    }
    lastError.clear();
    return true;
}

bool Formateur::supprimer(int idFormateur)
{
    if (!Session::exigerAdmin(&lastError)) {
        return false;
    }
    if (idFormateur <= 0) {
        lastError = QString::fromUtf8("Sélectionnez un formateur à supprimer.");
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM FORMATEUR WHERE ID_FORMATEUR = :id");
    query.bindValue(":id", idFormateur);
    if (!query.exec()) {
        setOracleError(query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        lastError = QString::fromUtf8("Formateur introuvable.");
        return false;
    }
    lastError.clear();
    return true;
}

QSqlQueryModel* Formateur::afficher()
{
    QSqlQuery query;
    query.prepare(QString(SQL_FORMATEUR_SELECT) + "ORDER BY f.ID_FORMATEUR");
    return executerSelect(query);
}

QSqlQueryModel* Formateur::rechercher(const QString& nomCritere,
                                      const QString& specialiteCritere,
                                      const QString& statutCritere)
{
    QString sql = QString(SQL_FORMATEUR_SELECT) + "WHERE 1=1 ";
    if (!nomCritere.trimmed().isEmpty()) {
        sql += "AND UPPER(f.NOM) LIKE UPPER(:nom) ";
    }
    if (!specialiteCritere.trimmed().isEmpty() && specialiteCritere != QLatin1String("Tous")) {
        sql += "AND f.SPECIALITE = :specialite ";
    }
    if (!statutCritere.trimmed().isEmpty() && statutCritere != QLatin1String("Tous")) {
        sql += "AND f.STATUT = :statut ";
    }
    sql += "ORDER BY f.NOM";

    QSqlQuery query;
    query.prepare(sql);
    if (!nomCritere.trimmed().isEmpty()) {
        query.bindValue(":nom", QString("%%1%").arg(nomCritere.trimmed()));
    }
    if (!specialiteCritere.trimmed().isEmpty() && specialiteCritere != QLatin1String("Tous")) {
        query.bindValue(":specialite", specialiteCritere.trimmed());
    }
    if (!statutCritere.trimmed().isEmpty() && statutCritere != QLatin1String("Tous")) {
        query.bindValue(":statut", statutCritere.trimmed());
    }
    return executerSelect(query);
}

QSqlQueryModel* Formateur::trierMulti(const QStringList& criteres, const QStringList& ordres)
{
    const QStringList colonnes = {
        QLatin1String("ID_FORMATEUR"), QLatin1String("NOM"), QLatin1String("PRENOM"),
        QLatin1String("SPECIALITE"), QLatin1String("STATUT"), QLatin1String("DATE_EMBAUCHE")
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
        orderParts << QString("f.%1 %2").arg(col, sens);
    }
    if (orderParts.isEmpty()) {
        orderParts << QStringLiteral("f.NOM ASC");
    }

    QSqlQuery query;
    query.prepare(QString(SQL_FORMATEUR_SELECT) + QString("ORDER BY %1").arg(orderParts.join(", ")));
    return executerSelect(query);
}

QSqlQueryModel* Formateur::trier(const QString& critere, const QString& ordre)
{
    return trierMulti(QStringList{critere}, QStringList{ordre});
}

QSqlQueryModel* Formateur::listePourCombo()
{
    QSqlQuery query;
    query.prepare("SELECT ID_FORMATEUR, NOM || ' ' || PRENOM || ' — ' || SPECIALITE AS LIBELLE "
                  "FROM FORMATEUR ORDER BY NOM, PRENOM");
    QSqlQueryModel* model = new QSqlQueryModel();
    if (!query.exec()) {
        setOracleError(query);
    }
    model->setQuery(std::move(query));
    return model;
}

QMap<QString, int> Formateur::statistiquesParSpecialite()
{
    QMap<QString, int> stats;
    QSqlQuery query;
    query.prepare("SELECT SPECIALITE, COUNT(*) AS NB FROM FORMATEUR GROUP BY SPECIALITE ORDER BY SPECIALITE");
    if (!query.exec()) {
        setOracleError(query);
        return stats;
    }
    while (query.next()) {
        stats.insert(query.value(0).toString(), query.value(1).toInt());
    }
    return stats;
}

QMap<QString, int> Formateur::statistiquesParStatut()
{
    QMap<QString, int> stats;
    QSqlQuery query;
    query.prepare("SELECT STATUT, COUNT(*) AS NB FROM FORMATEUR GROUP BY STATUT ORDER BY STATUT");
    if (!query.exec()) {
        setOracleError(query);
        return stats;
    }
    while (query.next()) {
        stats.insert(query.value(0).toString(), query.value(1).toInt());
    }
    return stats;
}

QSqlQueryModel* Formateur::chargePedagogique()
{
    QSqlQuery query;
    query.prepare(
        "SELECT f.ID_FORMATEUR, f.NOM, f.PRENOM, f.SPECIALITE, "
        "COUNT(c.ID_COURS) AS NB_COURS, NVL(SUM(c.DUREE_H),0) AS HEURES "
        "FROM FORMATEUR f "
        "LEFT JOIN COURS c ON c.ID_FORMATEUR = f.ID_FORMATEUR AND c.STATUT <> 'Annule' "
        "GROUP BY f.ID_FORMATEUR, f.NOM, f.PRENOM, f.SPECIALITE "
        "ORDER BY HEURES DESC, f.NOM");
    QSqlQueryModel* model = new QSqlQueryModel();
    if (!query.exec()) {
        setOracleError(query);
    }
    model->setQuery(std::move(query));
    model->setHeaderData(0, Qt::Horizontal, QString::fromUtf8("ID"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromUtf8("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromUtf8("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromUtf8("Spécialité"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromUtf8("Nb cours"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromUtf8("Heures"));
    return model;
}

QSqlQueryModel* Formateur::conflitsPlanning()
{
    QSqlQuery query;
    query.prepare(
        "SELECT a.ID_FORMATEUR, f.NOM || ' ' || f.PRENOM AS FORMATEUR, "
        "a.ID_COURS AS COURS_A, a.TITRE AS TITRE_A, "
        "TO_CHAR(a.DATE_DEBUT,'DD/MM/YYYY') AS DEBUT_A, TO_CHAR(a.DATE_FIN,'DD/MM/YYYY') AS FIN_A, "
        "b.ID_COURS AS COURS_B, b.TITRE AS TITRE_B, "
        "TO_CHAR(b.DATE_DEBUT,'DD/MM/YYYY') AS DEBUT_B, TO_CHAR(b.DATE_FIN,'DD/MM/YYYY') AS FIN_B "
        "FROM COURS a "
        "JOIN COURS b ON a.ID_FORMATEUR = b.ID_FORMATEUR AND a.ID_COURS < b.ID_COURS "
        "JOIN FORMATEUR f ON f.ID_FORMATEUR = a.ID_FORMATEUR "
        "WHERE a.STATUT <> 'Annule' AND b.STATUT <> 'Annule' "
        "AND a.DATE_DEBUT <= b.DATE_FIN AND b.DATE_DEBUT <= a.DATE_FIN "
        "ORDER BY f.NOM");
    QSqlQueryModel* model = new QSqlQueryModel();
    if (!query.exec()) {
        setOracleError(query);
    }
    model->setQuery(std::move(query));
    model->setHeaderData(0, Qt::Horizontal, QString::fromUtf8("ID formateur"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromUtf8("Formateur"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromUtf8("Cours A"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromUtf8("Titre A"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromUtf8("Début A"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromUtf8("Fin A"));
    model->setHeaderData(6, Qt::Horizontal, QString::fromUtf8("Cours B"));
    model->setHeaderData(7, Qt::Horizontal, QString::fromUtf8("Titre B"));
    model->setHeaderData(8, Qt::Horizontal, QString::fromUtf8("Début B"));
    model->setHeaderData(9, Qt::Horizontal, QString::fromUtf8("Fin B"));
    return model;
}

int Formateur::compter()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM FORMATEUR");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int Formateur::compterActifs()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM FORMATEUR WHERE STATUT = :statut");
    query.bindValue(":statut", QString::fromUtf8("Actif"));
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QString Formateur::htmlEntete(const QString& titre) const
{
    return QString::fromUtf8(
        "<div style='background-color:#0F2744;color:white;padding:18px 24px;'>"
        "<h1 style='margin:0;font-family:Arial;'>FormaPlus</h1>"
        "<p style='margin:4px 0 0 0;'>Centre de formation — GCentreFormation</p>"
        "</div>"
        "<h2 style='color:#0F2744;font-family:Arial;margin-top:20px;'>%1</h2>"
        "<p style='color:#555;font-family:Arial;'>Document généré le %2</p>")
        .arg(titre, QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm"));
}

QString Formateur::htmlPied() const
{
    return QString::fromUtf8(
        "<hr/><p style='font-size:10px;color:#777;font-family:Arial;'>"
        "Document officiel FormaPlus — usage interne du centre de formation. "
        "Ne pas photocopier sans en-tête personnalisé.</p>");
}

bool Formateur::ecrirePdf(const QString& html, const QString& chemin)
{
    QPdfWriter writer(chemin);
    writer.setResolution(96);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Portrait);
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    writer.setTitle(QString::fromUtf8("FormaPlus — Formateur"));

    const QString htmlPage = QString::fromUtf8(
        "<html><head><meta charset='utf-8'/>"
        "<style>"
        "body { font-family: Arial, sans-serif; font-size: 11pt; color: #222; }"
        "h1 { font-size: 18pt; margin: 0; }"
        "h2 { font-size: 14pt; color: #0F2744; }"
        "h3 { font-size: 12pt; color: #0F2744; }"
        "table { width: 100%; border-collapse: collapse; }"
        "th, td { padding: 6px; }"
        "</style></head><body>%1</body></html>").arg(html);

    QTextDocument doc;
    doc.setDocumentMargin(0);
    const QSizeF page = writer.pageLayout().paintRectPixels(writer.resolution()).size();
    doc.setPageSize(page);
    doc.setHtml(htmlPage);
    doc.print(&writer);
    lastError.clear();
    return true;
}

bool Formateur::exporterFichePdf(int idFormateur, const QString& chemin)
{
    QSqlQuery query;
    query.prepare(
        "SELECT ID_FORMATEUR, CIN, NOM, PRENOM, EMAIL, TELEPHONE, SPECIALITE, "
        "TO_CHAR(DATE_EMBAUCHE,'DD/MM/YYYY'), STATUT "
        "FROM FORMATEUR WHERE ID_FORMATEUR = :id");
    query.bindValue(":id", idFormateur);
    if (!query.exec() || !query.next()) {
        lastError = QString::fromUtf8("Formateur introuvable pour l'export PDF.");
        return false;
    }

    QSqlQuery cours;
    cours.prepare(
        "SELECT TITRE, NIVEAU, DUREE_H, PRIX, TO_CHAR(DATE_DEBUT,'DD/MM/YYYY'), "
        "TO_CHAR(DATE_FIN,'DD/MM/YYYY'), STATUT "
        "FROM COURS WHERE ID_FORMATEUR = :id ORDER BY DATE_DEBUT");
    cours.bindValue(":id", idFormateur);
    cours.exec();

    QString lignes;
    while (cours.next()) {
        lignes += QString::fromUtf8(
            "<tr><td>%1</td><td>%2</td><td align='right'>%3 h</td>"
            "<td align='right'>%4 DT</td><td>%5</td><td>%6</td><td>%7</td></tr>")
            .arg(cours.value(0).toString(),
                 cours.value(1).toString(),
                 cours.value(2).toString(),
                 QString::number(cours.value(3).toDouble(), 'f', 2),
                 cours.value(4).toString(),
                 cours.value(5).toString(),
                 cours.value(6).toString());
    }
    if (lignes.isEmpty()) {
        lignes = QString::fromUtf8("<tr><td colspan='7'>Aucun cours affecté.</td></tr>");
    }

    const QString html = htmlEntete(QString::fromUtf8("Fiche formateur"))
        + QString::fromUtf8(
            "<table width='100%' cellpadding='6' cellspacing='0' style='font-family:Arial;'>"
            "<tr><td><b>ID</b></td><td>%1</td><td><b>CIN</b></td><td>%2</td></tr>"
            "<tr><td><b>Nom</b></td><td>%3</td><td><b>Prénom</b></td><td>%4</td></tr>"
            "<tr><td><b>E-mail</b></td><td>%5</td><td><b>Téléphone</b></td><td>%6</td></tr>"
            "<tr><td><b>Spécialité</b></td><td>%7</td><td><b>Statut</b></td><td>%8</td></tr>"
            "<tr><td><b>Embauche</b></td><td colspan='3'>%9</td></tr>"
            "</table>"
            "<h3 style='color:#0F2744;font-family:Arial;'>Cours enseignés</h3>"
            "<table width='100%' border='1' cellspacing='0' cellpadding='5' "
            "style='border-collapse:collapse;font-family:Arial;font-size:12px;'>"
            "<tr style='background:#0F2744;color:white;'>"
            "<th>Titre</th><th>Niveau</th><th>Durée</th><th>Prix</th>"
            "<th>Début</th><th>Fin</th><th>Statut</th></tr>"
            "%10</table>")
            .arg(query.value(0).toString(), query.value(1).toString(),
                 query.value(2).toString(), query.value(3).toString(),
                 query.value(4).toString(), query.value(5).toString(),
                 query.value(6).toString(), query.value(8).toString(),
                 query.value(7).toString(), lignes)
        + htmlPied();

    return ecrirePdf(html, chemin);
}

bool Formateur::exporterListePdf(const QString& nomCritere,
                                const QString& specialiteCritere,
                                const QString& statutCritere,
                                const QString& chemin)
{
    QSqlQueryModel* model = rechercher(nomCritere, specialiteCritere, statutCritere);
    QString lignes;
    for (int i = 0; i < model->rowCount(); ++i) {
        lignes += QString::fromUtf8(
            "<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
            .arg(model->data(model->index(i, 0)).toString(),
                 model->data(model->index(i, 1)).toString(),
                 model->data(model->index(i, 2)).toString(),
                 model->data(model->index(i, 3)).toString(),
                 model->data(model->index(i, 6)).toString(),
                 model->data(model->index(i, 7)).toString(),
                 model->data(model->index(i, 8)).toString());
    }
    delete model;
    if (lignes.isEmpty()) {
        lignes = QString::fromUtf8("<tr><td colspan='7'>Aucun formateur.</td></tr>");
    }

    const QString html = htmlEntete(QString::fromUtf8("Liste des formateurs"))
        + QString::fromUtf8(
            "<p style='font-family:Arial;'>Filtres — nom : <b>%1</b> · spécialité : <b>%2</b> · statut : <b>%3</b></p>"
            "<table width='100%' border='1' cellspacing='0' cellpadding='5' "
            "style='border-collapse:collapse;font-family:Arial;font-size:12px;'>"
            "<tr style='background:#0F2744;color:white;'>"
            "<th>ID</th><th>CIN</th><th>Nom</th><th>Prénom</th>"
            "<th>Spécialité</th><th>Embauche</th><th>Statut</th></tr>"
            "%4</table>")
            .arg(nomCritere.isEmpty() ? QString::fromUtf8("tous") : nomCritere,
                 specialiteCritere.isEmpty() ? QString::fromUtf8("toutes") : specialiteCritere,
                 statutCritere.isEmpty() ? QString::fromUtf8("tous") : statutCritere,
                 lignes)
        + htmlPied();

    return ecrirePdf(html, chemin);
}
