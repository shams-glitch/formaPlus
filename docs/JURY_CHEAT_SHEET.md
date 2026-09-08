# Fiche jury — GCentreFormation (code réel)

Réponses courtes basées sur **ce dépôt** : `src/connection.*`, `src/formateur.*`, `src/cours.*`, `src/gcentreformation.*`, `sql/mpd_oracle.sql`, `docs/conception.md`.  
Sujet officiel : *Projet C++ — Session Repassage Septembre 2026*.

Arduino / marketing / teamwork : **N/A — NOT REQUIRED BY SEPTEMBER 2026 SUBJECT**.

---

## 1. Pourquoi avez-vous choisi Formateurs et Cours ?

Le sujet impose **deux modules** parmi Formateurs, Stagiaires, Cours, Salles. Formateurs + Cours sont liés par **enseigne (0,N)–(1,1)** : un cours a un formateur ; un formateur peut n’avoir aucun cours. Ça justifie la FK, la charge pédagogique, les conflits et l’affectation.

## 2. Pourquoi utiliser Qt ?

Sujet : « application desktop en utilisant le framework QT ». Widgets, SQL (`QODBC`), Charts, `QPdfWriter`. Un seul `.ui` (`ui/gcentreformation.ui`).

## 3. Pourquoi Oracle ?

Sujet : « Utiliser obligatoirement l’SGBD oracle ». Ici : Oracle XE, DSN ODBC **`Source_Projet2A`**, driver QODBC.

## 4. Comment fonctionne le Singleton ?

`Connection::instance()` crée **une** instance (`p_instance`). Constructeur privé, copie et affectation interdites (`= delete`). Tous les `QSqlQuery` réutilisent la connexion Qt par défaut.

## 5. Pourquoi une seule connexion ?

Imposé par le sujet. Une seule `QSqlDatabase` QODBC ouverte dans `createConnect()`, fermée dans le destructeur. `main.cpp` appelle `Connection::instance()->createConnect()`.

## 6. Où se trouve le SQL ?

Uniquement dans les classes pures **`Formateur`** et **`Cours`**. Identifiants de connexion dans `Connection` (env / `connection.ini`, pas dans le source).

## 7. Pourquoi ne pas mettre SQL dans les boutons ?

Sujet : « Aucune requête ne doit être implémentée derrière un bouton ». Les slots (`on_btnFormAjouter_clicked`, etc.) appellent le modèle puis `chargerTout()`. Séparation GUI / accès données.

## 8. Pourquoi utiliser `prepare()` ?

`QSqlQuery::prepare("… :nom …")` envoie le **texte SQL** à Oracle **sans** coller les valeurs. Ex. `INSERT INTO FORMATEUR … VALUES (:cin, :nom, …)`.

## 9. Pourquoi `bindValue()` ?

Lie une valeur à un placeholder : `query.bindValue(":cin", cin);`. La valeur n’est pas concaténée dans la chaîne SQL.

## 10. Comment évitez-vous l’injection SQL ?

`prepare()` + `bindValue()` pour toute valeur utilisateur. Le `ORDER BY` n’accepte que des colonnes d’une **liste blanche** + `ASC`/`DESC`.

## 11. Comment fonctionne le CRUD ?

`ajouter()`, `afficher()`, `modifier()`, `supprimer(id)` dans `Formateur` / `Cours`. Validation `valider()` puis requête préparée. ID auto : trigger + séquence.

## 12. Comment fonctionne la recherche multi-critères ?

`Formateur::rechercher(nom, specialite, statut)` : `LIKE :nom` / `= :specialite` / `= :statut`.  
`Cours::rechercher(titre, niveau, statut, idFormateur)` : ≥ 3 critères.

## 13. Comment fonctionne le tri ?

`trierMulti()` : colonnes autorisées seulement (formateur : NOM, SPECIALITE, STATUT… ; cours : TITRE, NIVEAU, STATUT…). Sens `ASC`/`DESC` uniquement.

## 14. Comment fonctionnent les statistiques ?

`GROUP BY` : spécialité/statut formateurs ; niveau/formateur/mois cours. Accueil : `compter()`, `compterActifs()`, `compterParStatut()`, `sommeHeures()`.

## 15. Comment les graphiques sont-ils actualisés ?

Après **chaque** CRUD réussi : `chargerTout()` → `rafraichirStatistiques()` recrée les `QChartView` (`QPieSeries` / `QBarSeries`). L’accueil aussi (`rafraichirAccueil()`).

## 16. Comment fonctionne la génération PDF ?

`QPdfWriter` + HTML (en-tête FormaPlus, tableaux, pied). **Pas** une capture d’écran. Un seul type : PDF. Fiche formateur / liste ; fiche cours / catalogue.

## 17. Quelle est la relation Formateur/Cours ?

Association **enseigne** : FORMATEUR **(0,N)** — COURS **(1,1)**. Au MLD, `COURS.id_formateur` est FK vers `FORMATEUR`.

## 18. Qu’est-ce qu’une PK ?

Clé primaire : identifiant unique d’une ligne. Ici `ID_FORMATEUR`, `ID_COURS` (séquences + triggers `BEFORE INSERT`).

## 19. Qu’est-ce qu’une FK ?

Clé étrangère : `COURS.ID_FORMATEUR` référence `FORMATEUR.ID_FORMATEUR` (`FK_COURS_FORMATEUR`). Un cours doit pointer vers un formateur existant.

## 20. Comment gérez-vous les contraintes Oracle ?

PK / UNIQUE (CIN, e-mail) / CHECK (CIN 8 chiffres, statuts, dates, prix) / FK. En C++ : `setOracleError()` traduit `ORA-00001`, `ORA-02290`, `ORA-02292`.

## 21. Que se passe-t-il si on supprime un Formateur utilisé par un Cours ?

Oracle refuse : **ORA-02292** (pas de `ON DELETE CASCADE`). Message métier : des cours sont encore rattachés.

## 22. Pourquoi une validation côté C++ et côté Oracle ?

Sujet : contrôles dans le **code C++ et dans la base**. `valider()` (CIN, e-mail, dates, prix > 0…) **et** CHECK/UNIQUE/FK. Double protection.

## 23. Comment est organisée l’interface Qt ?

Un seul `.ui` : sidebar + `QStackedWidget` (Accueil / Formateurs / Cours) + `QTabWidget`. Peu de dialogs (`QMessageBox` surtout erreurs / confirmation).

## 24. Comment utilisez-vous Git/GitHub ?

Dépôt `https://github.com/shams-glitch/formaPlus.git`, branche `main`. AA3.2 A = montrer `git log`, `git remote -v`, un **push** et un **pull --ff-only`. Pas d’historique inventé.

## 25. Pourquoi Arduino n’est-il pas présent ?

Arduino ne fait pas partie des exigences du sujet officiel de
repassage de septembre 2026. Nous nous sommes donc concentrés sur les fonctionnalités demandées par le sujet.

---

## Connexion (si demandé)

1. `Connection::instance()->createConnect()` après `QApplication`.
2. `addDatabase("QODBC")`.
3. DSN / utilisateur / mot de passe via `ORACLE_*` ou `connection.ini` à côté de l’exe (**pas** dans le code).
4. Aucun mot de passe dans les journaux (`(vide)` / `(fourni)`).
5. `closeConnection()` dans le destructeur.

## Métiers libres

- Formateurs : **charge pédagogique**, **conflits de planning**.
- Cours : **affectation** (`affecterFormateur`), **alertes** (7 jours, formateur inactif, chevauchement).

## Démo Git

```text
git log --oneline --decorate
git status
git remote -v
# origin  https://github.com/shams-glitch/formaPlus.git
git pull --ff-only origin main
```
