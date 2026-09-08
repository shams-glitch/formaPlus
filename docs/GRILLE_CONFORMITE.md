# Conformité à la grille critériée — Repassage PCPP 2026

Preuves concrètes pour chaque acquis. À utiliser à l’oral (AA7.1).

| Acquis | Pour A (5 pts) | Preuve dans le projet | Statut |
|--------|----------------|------------------------|--------|
| **AA2.1 MCD** | Associations, cardinalités, propriétés + PK | `docs/conception.md` §1 — enseigne (0,N)–(1,1), PK formateur/cours | OK |
| **AA2.2 MLD** | Relations + intégrité depuis MCD | `docs/conception.md` §2 — FK `#id_formateur`, UNIQUE, domaines | OK |
| **AA2.3 MPD** | Script Oracle cohérent | `sql/mpd_oracle.sql` — PK/FK/CHECK/séquences/triggers/index | OK |
| **AA3.2 Git** | commit + **push** + **pull** réguliers | 4 commits locaux (`git log`) | **À finaliser** : push GitHub |
| **AA4.1 CRUD** | 4 ops + contrôles + cohérence | `formateur.cpp` / `cours.cpp` + `valider()` + CHECK Oracle | OK |
| **AA4.2 Métiers** | 5 métiers / module + contrôles | Voir tableau métiers ci-dessous | OK |
| **AA4.3 GUI** | Ergonomique, navigation fluide | 1 `.ui`, sidebar + Stacked + Tabs, peu de dialogs | OK |
| **AA7.1 Oral** | Réponses précises | `docs/guide_soutenance.md` | À répéter |

---

## Checklist sujet (énoncé)

| Exigence | Respectée ? | Où |
|----------|-------------|-----|
| 2 modules | Oui | Formateurs + Cours |
| CRUD × 2 | Oui | Onglets « Liste & CRUD » |
| Git + GitHub | Partiel | Commits OK — **push GitHub manquant** |
| Recherche + tri ≥ 3 critères / module | Oui | Formateurs : nom, spécialité, statut ; Cours : titre, niveau, statut (+ formateur) ; tri jusqu’à 3 colonnes |
| Stats + graphiques live | Oui | Qt Charts ; `chargerTout()` après chaque CRUD |
| 1 seul type de document | Oui | PDF uniquement |
| PDF personnalisé (pas screenshot) | Oui | En-tête FormaPlus, tableaux, pied de page (`QPdfWriter`) |
| 2 métiers libres / module | Oui | Charge + conflits ; Affectation + alertes |
| Intégrité PK/FK | Oui | Script Oracle + messages `ORA-02292` |
| Une seule connexion | Oui | Singleton `Connection::instance()` |
| Oracle | Oui | QODBC `Source_Projet2A` |
| Requêtes préparées | Oui | `prepare()` + `bindValue()` dans les modèles |
| GUI containers, peu de dialog | Oui | Stacked Widget + Tab Widget ; MessageBox surtout erreurs |
| Contrôles C++ **et** BD | Oui | `valider()` + CHECK/UNIQUE/FK |
| Requêtes dans classes C++ pures | Oui | `Formateur`, `Cours` |
| Aucune requête derrière un bouton | Oui | Slots appellent seulement le modèle |

---

## 5 métiers × 2 modules (AA4.2 = A)

### Formateurs
1. Recherche / tri multicritères  
2. Stats graphiques live  
3. Export PDF  
4. Charge pédagogique  
5. Conflits de planning  

### Cours
1. Recherche / tri multicritères  
2. Stats graphiques live  
3. Export PDF  
4. Affectation formateur  
5. Alertes  

---

## Ce qui reste avant la soutenance (critique)

1. **Créer le dépôt GitHub** et pousser (`push`) — sans ça AA3.2 ≠ A.  
2. Exécuter `sql/mpd_oracle.sql` + DSN ODBC `Source_Projet2A`.  
3. Compléter le Word `docs/Cahier_Specification_GCentreFormation.docx` : table des matières auto + captures GUI.  
4. Répéter l’oral avec `docs/guide_soutenance.md`.
