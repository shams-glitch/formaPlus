# GCentreFormation — FormaPlus

Application desktop **Qt / C++** de gestion d’un centre de formation.
Modules : **Formateurs** et **Cours**. SGBD : **Oracle**. Patron : **singleton** + classes C++ pures.

## Prérequis

1. **Qt Creator** (qmake, modules `sql`, `charts`, `printsupport`)
2. **Oracle XE** + **SQL Developer**
3. Source ODBC Windows nommée **`Source_Projet2A`** (atelier officiel)

## Base de données

1. Créer un utilisateur Oracle, par exemple :

```sql
CREATE USER cpp IDENTIFIED BY cpp;
GRANT CONNECT, RESOURCE TO cpp;
ALTER USER cpp QUOTA UNLIMITED ON USERS;
```

2. Dans SQL Developer, exécuter `sql/mpd_oracle.sql` (tables, contraintes, séquences, jeu d’essai).

3. Adapter `connection.cpp` si besoin :

```cpp
db.setDatabaseName("Source_Projet2A");
db.setUserName("cpp");
db.setPassword("cpp");
```

4. Créer la source ODBC 64 bits : *Sources de données ODBC* → Oracle → nom exact `Source_Projet2A`.

## Lancer le projet

1. Ouvrir `GCentreFormation.pro` dans Qt Creator
2. Kit Desktop Qt 5.15+ ou Qt 6, compilateur 64 bits (même bitness qu’Oracle)
3. Compiler avec l’icône **grise**, exécuter avec l’icône **verte**

## Structure du code

| Dossier / fichier | Contenu |
|-------------------|---------|
| `src/` | C++ : `main`, `connection` (singleton), `formateur`, `cours`, `gcentreformation` |
| `ui/` | `gcentreformation.ui` (1 seul fichier UI) |
| `sql/` | `mpd_oracle.sql` |
| `docs/` | Conception, grille, guide, cahier Word |
| `GCentreFormation.pro` | Projet qmake |

## Architecture (atelier CRUD)

| Couche | Fichiers | Rôle |
|--------|----------|------|
| Connexion | `src/connection.h/.cpp` | Une seule connexion QODBC (singleton) |
| Modèle | `src/formateur.*`, `src/cours.*` | Toutes les requêtes préparées |
| Vue | `ui/gcentreformation.ui` + `src/gcentreformation.*` | Stacked Widget + Tab Widget |

Aucune requête SQL n’est écrite derrière un bouton.

## Fonctionnalités

**Formateurs** — CRUD, recherche/tri (nom, spécialité, statut), stats live, fiche/liste PDF, charge pédagogique, conflits de planning.

**Cours** — CRUD, recherche/tri (titre, niveau, statut, formateur), stats live, fiche + catalogue PDF, affectation formateur, alertes.

## Conception

- MCD / MLD / MPD : `docs/conception.md`
- **Conformité grille** : `docs/GRILLE_CONFORMITE.md`
- Guide oral : `docs/guide_soutenance.md`
- Cahier de spécification Word : `docs/Cahier_Specification_GCentreFormation.docx`  
  (Régénérer : `py -3.12 scripts/generate_cahier.py`)

## Git

```bash
cd GCentreFormation
git init
git add .
git commit -m "Version initiale GCentreFormation — Formateurs et Cours"
git branch -M main
git remote add origin <URL-GitHub>
git push -u origin main
```

Faire des commits réguliers (CRUD, métiers, GUI) pour valider AA3.2.
