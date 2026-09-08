# Checklist officielle — Repassage PCPP septembre 2026

Source : `01_sujet/Sujet_repassage_Septembre_2026.pdf` et `01_sujet/Grille_criteriee_PCPP_Repassage_2026.pdf`.

Légende : `PASS` | `FAIL` | `PARTIAL` | `NOT TESTED` | `N/A — NOT REQUIRED`

Arduino / marketing / teamwork : **N/A — NOT REQUIRED BY SEPTEMBER 2026 SUBJECT**.

| Exigence | Implémentation | Test | Résultat | Preuve | Action restante |
|----------|----------------|------|----------|--------|-----------------|
| Qt/C++ qmake | `.pro` sql charts printsupport widgets | mingw32-make release | PASS | Qt 6.8.3 + g++ 13.1.0 | — |
| 2 modules | Formateurs + Cours | GUI sidebar | PASS | pages ouvertes | — |
| CRUD Formateurs | `Formateur::*` | GUI A/R/U/D | PASS | messages + Oracle 8 | — |
| CRUD Cours | `Cours::*` | GUI add + delete Yes | PASS | *Cours ajouté.* / *Cours supprimé.* ; COUNT=12 | — |
| Recherche ≥ 3 | `rechercher()` bindValue | GUI + SQL | PASS | nom/spécialité/statut ; titre/niveau/statut | — |
| Tri | `trierMulti` whitelist | code + combos | PASS | `formateur.cpp` / `cours.cpp` | — |
| Stats + charts live | Qt Charts + `chargerTout()` | GUI | PASS | Accueil KPI + QChartView | — |
| PDF personnalisé | `QPdfWriter` | fichiers générés | PASS | fiche + catalogue | — |
| Métiers Formateurs | charge + conflits | GUI + SQL | PASS | heures ; overlap | — |
| Métiers Cours | affectation + alertes | SQL + GUI | PASS | `affecterFormateur` ; alertes | — |
| Intégrité Oracle | PK/FK/CHECK/seq/trig | sqlplus (pas de DROP) | PASS | FK_COURS_FORMATEUR ; ORA-02292 | Ne pas relancer `mpd_oracle.sql` |
| Singleton | `Connection::instance()` | code | PASS | copie / affectation `= delete` | — |
| Connexion DSN | QODBC + ini/env | lancement | PASS | pas de dialog erreur | `connection.ini` gitignored |
| Requêtes préparées | prepare/bindValue | revue source | PASS | pas de SQL dans les slots | — |
| Validation saisie | `valider()` | GUI CIN | PASS | 8 chiffres | — |
| MCD/MLD/MPD | `conception.md` | relecture | PASS | (0,N)–(1,1) | — |
| Git | identity + commit + push + pull | `main` / formaPlus | PASS | `4b4c0da` + commit présentation | — |
| Présentation jury | 10 slides PPTX/PDF | fichier + captures réelles | PASS | `docs/GCentreFormation_Presentation_Septembre_2026.pptx` | — |
| Arduino | — | — | N/A — NOT REQUIRED BY SEPTEMBER 2026 SUBJECT | énoncé | — |
| Marketing | — | — | N/A — NOT REQUIRED BY SEPTEMBER 2026 SUBJECT | énoncé | — |
| Teamwork | — | — | N/A — NOT REQUIRED BY SEPTEMBER 2026 SUBJECT | énoncé | — |
| Cahier Word | script | fichier présent | PARTIAL | `.docx` présent | TOC + captures manuelles |

**Sécurité :** le source actuel n’a plus de mot de passe en dur. L’ancien commit public `fbaacb0` en contenait un. Rotater le mot de passe Oracle applicatif. Ne pas réécrire l’historique Git.
