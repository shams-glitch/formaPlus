# Conception — GCentreFormation

Application desktop Qt/C++ de gestion d’un centre de formation.
Modules retenus : **Gestion des Formateurs** et **Gestion des Cours**.

---

## 1. MCD (Modèle Conceptuel de Données) — AA2.1

### Association et cardinalités

```text
                         enseigne
  FORMATEUR (0,N)  <-------------------->  (1,1) COURS
```

| Règle | Formulation Merise |
|-------|--------------------|
| Côté FORMATEUR | Un formateur enseigne **0 à N** cours |
| Côté COURS | Un cours est enseigné par **exactement 1** formateur |
| Association | **enseigne** (binaire) |

### Schéma MCD (notation Merise)

```text
  ┌─────────────────┐         enseigne          ┌─────────────────┐
  │   FORMATEUR     │ 0,N                   1,1 │     COURS       │
  │─────────────────│◄─────────────────────────►│─────────────────│
  │ id_formateur PK │                           │ id_cours PK     │
  │ cin             │                           │ titre           │
  │ nom             │                           │ niveau          │
  │ prenom          │                           │ duree_h         │
  │ email           │                           │ prix            │
  │ telephone       │                           │ date_debut      │
  │ specialite      │                           │ date_fin        │
  │ date_embauche   │                           │ statut          │
  │ statut          │                           └─────────────────┘
  └─────────────────┘
```

> Au niveau **MCD**, la clé étrangère n’apparaît pas : elle naît au passage MCD → MLD.

### Propriétés de FORMATEUR (avec clé primaire)

| Propriété     | Type conceptuel | Contrainte        |
|---------------|-----------------|-------------------|
| **id_formateur** | Identifiant   | **Clé primaire**  |
| cin           | Texte           | Unique, obligatoire |
| nom           | Texte           | Obligatoire       |
| prenom        | Texte           | Obligatoire       |
| email         | Texte           | Unique            |
| telephone     | Texte           | Obligatoire       |
| specialite    | Texte           | Obligatoire       |
| date_embauche | Date            | Obligatoire       |
| statut        | Actif / Inactif | Obligatoire       |

### Propriétés de COURS (avec clé primaire)

| Propriété    | Type conceptuel | Contrainte                          |
|--------------|-----------------|-------------------------------------|
| **id_cours** | Identifiant     | **Clé primaire**                    |
| titre        | Texte           | Obligatoire                         |
| niveau       | Enuméré         | Debutant / Intermediaire / Avance   |
| duree_h      | Entier          | > 0                                 |
| prix         | Décimal         | > 0                                 |
| date_debut   | Date            | Obligatoire                         |
| date_fin     | Date            | ≥ date_debut                        |
| statut       | Enuméré         | Planifie / En cours / Termine / Annule |

### Association **enseigne** (détail AA2.1)

| Extrémité | Entité   | Cardinalité | Signification |
|-----------|-----------|-------------|---------------|
| 1 | FORMATEUR | **0,N** | Un formateur peut n’avoir aucun cours ou plusieurs |
| 2 | COURS     | **1,1** | Chaque cours a un et un seul formateur |

---

## 2. MLD (Modèle Logique de Données) — AA2.2

Passage MCD → MLD : l’association **enseigne (0,N — 1,1)** est portée par COURS.

```text
FORMATEUR (#id_formateur, cin, nom, prenom, email, telephone,
           specialite, date_embauche, statut)

COURS (#id_cours, titre, niveau, duree_h, prix, date_debut, date_fin,
       statut, #id_formateur)
```

### Contraintes d’intégrité (MLD)

| Type | Règle |
|------|--------|
| PK | `FORMATEUR.id_formateur`, `COURS.id_cours` |
| FK | `COURS.id_formateur` → `FORMATEUR.id_formateur` |
| UNIQUE | `FORMATEUR.cin`, `FORMATEUR.email` |
| Domaine | statut, niveau, dates, prix > 0, formats CIN / e-mail / téléphone |
| Référentielle | Suppression d’un formateur **refusée** s’il a encore des cours (comportement restrict Oracle par défaut) |

---

## 3. MPD (Modèle Physique — Oracle) — AA2.3

Script : `sql/mpd_oracle.sql`

| Élément Oracle | Détail |
|----------------|--------|
| Types | `NUMBER`, `VARCHAR2`, `DATE` |
| PK / FK / UNIQUE | `PK_FORMATEUR`, `PK_COURS`, `FK_COURS_FORMATEUR`, `UQ_FORMATEUR_CIN`, `UQ_FORMATEUR_EMAIL` |
| CHECK | CIN 8 chiffres, téléphone, e-mail, noms, statuts, niveaux, dates, prix |
| Séquences | `SEQ_FORMATEUR`, `SEQ_COURS` |
| Triggers | `TRG_FORMATEUR_BI`, `TRG_COURS_BI` (auto-ID) |
| Index | `IDX_COURS_FORMATEUR`, `IDX_COURS_STATUT`, `IDX_FORM_SPECIALITE`, `IDX_FORM_STATUT` |

> Oracle n’accepte **pas** `ON DELETE RESTRICT`. Sans clause `ON DELETE`, Oracle refuse la suppression du parent s’il existe des enfants (`ORA-02292`).

---

## 4. Fonctionnalités (AA4.1 / AA4.2)

### Formateurs — CRUD + 5 métiers

| # | Métier | Critères / contenu |
|---|--------|--------------------|
| 1 | Recherche + tri multicritères | ≥ 3 critères : nom, spécialité, statut (+ tri jusqu’à 3 colonnes) |
| 2 | Statistiques graphiques | Par spécialité / statut — rafraîchies après chaque CRUD |
| 3 | Document PDF | Fiche formateur + liste filtrée (personnalisé, pas capture) |
| 4 | Charge pédagogique | Nb cours + heures (hors annulés) |
| 5 | Conflits de planning | Même formateur, dates qui se chevauchent |

### Cours — CRUD + 5 métiers

| # | Métier | Critères / contenu |
|---|--------|--------------------|
| 1 | Recherche + tri multicritères | ≥ 3 critères : titre, niveau, statut, formateur |
| 2 | Statistiques graphiques | Par niveau / formateur / mois — live après CRUD |
| 3 | Document PDF | Fiche cours + catalogue |
| 4 | Affectation formateur | Réaffectation d’un cours |
| 5 | Alertes | Début imminent, formateur inactif, chevauchement |
