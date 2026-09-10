# GCentreFormation — FormaPlus

Application desktop **Qt / C++** de gestion d’un centre de formation.
Modules : **Formateurs** et **Cours**. SGBD : **Oracle**. Patron : **singleton** + classes C++ pures.

## Prérequis

1. **Qt Creator** (qmake, modules `sql`, `charts`, `printsupport`)
2. **Oracle XE** + **SQL Developer**
3. Source ODBC Windows nommée **`Source_Projet2A`** (atelier officiel)

## Base de données

1. Créer un utilisateur Oracle (atelier), puis **ne pas** relancer `sql/mpd_oracle.sql` si les tables existent déjà (le script DROP).

2. Source ODBC 64 bits nommée exactement **`Source_Projet2A`**.

3. Identifiants **hors du code** : variables `ORACLE_DSN` / `ORACLE_USER` / `ORACLE_PASSWORD`, ou fichier `connection.ini` à côté de l’exe (voir `connection.ini.example`, **gitignored**).

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
| `docs/` | Conception, grille, guide, cahier Word, présentation jury |
| `GCentreFormation.pro` | Projet qmake |

## Architecture (atelier CRUD)

| Couche | Fichiers | Rôle |
|--------|----------|------|
| Connexion | `src/connection.h/.cpp` | Une seule connexion QODBC (singleton) |
| Modèle | `src/formateur.*`, `src/cours.*` | Toutes les requêtes préparées |
| Vue | `ui/gcentreformation.ui` + `src/gcentreformation.*` | Stacked Widget + Tab Widget |

Aucune requête SQL n’est écrite derrière un bouton.

## Authentification (fonctionnalité additionnelle)

L’application affiche un écran de connexion **avant** la fenêtre principale.
Un login réussi ouvre GCentreFormation. Un échec reste sur l’écran de connexion.

Ce n’est **pas** une exigence du sujet de septembre 2026.

- Le login applicatif est distinct de la connexion Oracle (DSN / `connection.ini`).
- Les mots de passe applicatifs ne sont pas stockés en clair : hachage PBKDF2 (SHA-256) dans `APP_UTILISATEUR`.
- Aucun identifiant n’est écrit dans le code source.
- Premier compte : copier `app_auth.local.ini.example` vers `app_auth.local.ini` à côté de l’exe (fichier **gitignored**), renseigner identifiant et mot de passe, puis lancer une fois.

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

Remote : `https://github.com/shams-glitch/formaPlus.git`  
Fiche orale : `docs/JURY_CHEAT_SHEET.md` · checklist : `docs/VALIDATION_CHECKLIST.md`  
Présentation : `docs/GCentreFormation_Presentation_Septembre_2026.pptx`

Arduino / marketing / teamwork : **N/A — NOT REQUIRED BY SEPTEMBER 2026 SUBJECT**.
