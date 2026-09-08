# Guide de soutenance — AA7.1 (persuader le jury)

## Architecture (réponses types)

**Q : Où sont les requêtes SQL ?**  
Dans les classes pures C++ `Formateur` et `Cours` uniquement. Les boutons appellent `ajouter()`, `afficher()`, `rechercher()`, etc. Aucune requête dans `gcentreformation.cpp`.

**Q : Pourquoi un Singleton ?**  
Le sujet impose **une seule connexion**. `Connection::instance()` crée une unique instance QODBC et la réutilise.

**Q : Pourquoi des requêtes préparées ?**  
Contre l’injection SQL (OWASP). `prepare()` + `bindValue()` : la partie variable n’est pas interprétée comme du SQL.

**Q : qmake ou CMake ?**  
**qmake**, recommandé dans les bonnes pratiques ESPRIT.

**Q : Pourquoi un seul `.ui` ?**  
Bonnes pratiques : un fichier UI + `Stacked Widget` / `Tab Widget` pour éviter les problèmes d’intégration et limiter les dialogs.

---

## Données (AA2)

**Q : Cardinalités de l’association enseigne ?**  
FORMATEUR **(0,N)** — COURS **(1,1)**. Un cours a exactement un formateur ; un formateur peut avoir zéro ou plusieurs cours.

**Q : Comment passe-t-on du MCD au MLD ?**  
L’association (0,N)–(1,1) est portée par COURS : on ajoute `#id_formateur` (FK).

**Q : Pourquoi pas `ON DELETE RESTRICT` dans le script Oracle ?**  
Oracle n’accepte pas cette syntaxe. Sans clause `ON DELETE`, le comportement est déjà restrict → `ORA-02292` si on supprime un formateur qui a des cours.

**Q : Contrôles C++ et BD ?**  
`valider()` (CIN 8 chiffres, e-mail, dates…) + CHECK / UNIQUE / FK Oracle. Double protection.

---

## Métiers (AA4.2)

**Q : Les graphiques sont-ils live ?**  
Oui. Après chaque CRUD réussi, `chargerTout()` → `rafraichirStatistiques()` recrée les Qt Charts.

**Q : Le PDF est-il personnalisé ?**  
Oui : en-tête FormaPlus, titre, date, tableaux HTML, pied de page via `QPdfWriter` — pas une capture d’écran.

**Q : Un seul type de document ?**  
PDF uniquement (sujet : un seul format).

**Q : Montrez un métier utile hors basiques.**  
Formateurs : charge pédagogique / conflits. Cours : affectation / alertes (début imminent, formateur inactif).

---

## Git (AA3.2)

**Q : Montrez l’historique.**  
`git log --oneline --decorate` sur `main` / `origin/main`. Remote : `https://github.com/shams-glitch/formaPlus.git`.  
Ne pas inventer de commits.

**Pour A** : montrer `git remote -v`, que `main` est déjà poussé, puis `git pull --ff-only origin main`.

**Q : Arduino / marketing / teamwork ?**  
N/A — NOT REQUIRED BY SEPTEMBER 2026 SUBJECT. Pas de `QSerialPort` dans ce projet.

---

## Démo rapide (ordre conseillé)

1. Accueil (KPI + graphiques)  
2. Formateurs : ajouter → table + stats se mettent à jour  
3. Recherche 3 critères + tri multicritères  
4. PDF fiche formateur  
5. Cours : CRUD + affectation + alertes  
6. Tenter de supprimer un formateur avec des cours → erreur d’intégrité  
