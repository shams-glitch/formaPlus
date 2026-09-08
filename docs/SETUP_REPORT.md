# Environment Setup Report — ESPRIT PCPP Repassage 2026

Generated: 2026-09-07

## === SYSTEM ===

| Item | Value |
|------|--------|
| Windows | Microsoft Windows 11 Famille Unilingue (10.0.26200) |
| Architecture | 64-bit |
| CPU | AMD Ryzen 5 5600H (6 cores / 12 threads) |
| RAM | 13.9 GB |
| Disk C: | ~263 GB free / 475 GB |

## === INSTALLED ===

| Tool | Status | Location / version |
|------|--------|--------------------|
| Git | OK | 2.53.0.windows.3 — `C:\Program Files\Git\cmd\git.exe` |
| Qt | OK | **6.8.3** MinGW 64-bit — `C:\Qt\6.8.3\mingw_64` |
| Qt Creator | OK | `C:\Qt\Tools\QtCreator\bin\qtcreator.exe` |
| Compiler | OK | MinGW-w64 g++ **13.1.0** — `C:\Qt\Tools\mingw1310_64` |
| CMake | OK | **4.4.3** — `C:\Program Files\CMake\bin\cmake.exe` (not required by this project) |
| Ninja | Present | PySchool Scripts (optional) |
| Arduino IDE | OK | 2.3.10 (winget) |
| Database | OK | **Oracle Database 11g Express Edition** — `C:\oraclexe` |
| DB services | OK | `OracleServiceXE` Running, `OracleXETNSListener` Running |
| Database client | OK | SQL Developer 24.3.1 + sqlplus |
| ODBC driver | OK | **Oracle in XE** (64-bit) |
| ODBC DSN | OK | **Source_Projet2A** (User, 64-bit) |
| Qt SQL driver | OK | `qsqlodbc.dll` present |

## === PROJECT ===

| Item | Value |
|------|--------|
| Path | `c:\Users\ctaye\OneDrive\Bureau\projet_cpp\GCentreFormation` |
| Build system | **qmake** (`.pro`) — **not CMake** |
| Qt modules | `core gui widgets sql charts printsupport` |
| C++ standard | C++17 |
| Database | **Oracle** (subject requirement) via **QODBC** |
| Connection | Singleton `Connection` → DSN `Source_Projet2A`, user `cpp` / `cpp` |
| Arduino in this repo | Not part of `GCentreFormation` sources; ateliers copied to `docs\arduino_ateliers\` |
| Main deps | Qt Charts, Qt SQL ODBC, Oracle XE |

## === TESTS ===

| Test | Result | Notes |
|------|--------|-------|
| C++ compilation | **PASS** | `qmake` + `mingw32-make` — exe built |
| Qt application | **PASS*** | Runs from `C:\Dev\GCentreFormation\` |
| Database connection | **PASS** | ODBC query `SELECT COUNT(*) FROM FORMATEUR` → **8** |
| Schema load | **PASS** | `mpd_oracle.sql` applied; FORMATEUR=8, COURS=12 |
| Arduino detection | **NOT TESTED** | No COM / serial ports; no board connected |
| Git | **PARTIAL** | Repo OK (5 commits); identity + remote missing |

\* OneDrive path is **blocked by Device Guard**. Deployed runnable copy to `C:\Dev\GCentreFormation\`.

## === CREDENTIALS / CONFIG (local) ===

| Item | Value |
|------|--------|
| Oracle SYSTEM password | `oraclexe` (from XE response file) |
| App schema | user `cpp` / password `cpp` |
| DSN | `Source_Projet2A` |
| Listener | localhost:1521, service **XE** |

## === PATH (User) UPDATED ===

Added:
- `C:\Qt\6.8.3\mingw_64\bin`
- `C:\Qt\Tools\mingw1310_64\bin`
- `C:\Program Files\CMake\bin`

Open a **new** terminal for PATH to apply everywhere.

## === REMAINING ACTIONS (manual) ===

1. **Git identity** (do not invent — set your real data):
   ```powershell
   git config --global user.name "YOUR_NAME"
   git config --global user.email "YOUR_EMAIL"
   ```
2. **GitHub Classroom** — add remote and `git push` / `git pull` (needed for AA3.2 grade A).
3. **Qt Creator kit** — first open: Preferences → Kits → ensure Desktop Qt 6.8.3 MinGW 64-bit uses:
   - Qt: `C:\Qt\6.8.3\mingw_64`
   - Compiler: MinGW `C:\Qt\Tools\mingw1310_64\bin`
   - Then open `GCentreFormation.pro`
4. **Device Guard** — run the app from `C:\Dev\GCentreFormation\GCentreFormation.exe` or via Qt Creator; OneDrive `release\` may stay blocked.
5. **Arduino hardware** — plug the board, check Device Manager → Ports (COM), then open ateliers in `docs\arduino_ateliers\`.
6. **Cahier Word** — add auto TOC + GUI screenshots before soutenance.

## === RUNNING NOW ===

- GCentreFormation (from `C:\Dev\...`)
- Qt Creator
- SQL Developer
- Arduino IDE
- Oracle XE services
