# CLAUDE.md — Daoa Mini (Cardputer ADV)

Claude Code est l'agent de développement principal de ce projet. Lire ce
fichier avant toute décision d'implémentation, puis `docs/01_DECISIONS.md`
(décisions actées) et `docs/05_ROADMAP.md` (état d'avancement).

## Ce qu'est ce projet

Daoa Mini est un **petit outil promotionnel de niche** pour l'app complète
Daoa (iPhone/Android) : un tirage Yi Jing ligne à ligne, hors-ligne, sur
M5Stack **Cardputer ADV** (ESP32-S3, écran 240 × 135, clavier 56 touches).

La boucle produit complète tient en quatre écrans :
accueil → tirage ligne à ligne → résultat (figure + complémentaire + texte
court FR/EN) → écran promo avec QR code vers l'app complète.

## Garde-fous de périmètre (fermes)

- Tirage **ligne à ligne uniquement**. Pas de tirage par cartes.
- **Pas** de question en entrée, pas de saisie de texte.
- **Pas** de journal, pas d'IA, pas de backend, pas de réseau connecté,
  pas de crédits/paiement, pas de comptes.
- **Hors-ligne au sens strict** (amendé 2026-08-09) : la seule exception
  est la « lecture du lieu » — un **scan WiFi passif** avant le tirage.
  Jamais de connexion, radio coupée sitôt le scan fini, rien n'est
  stocké ni transmis, aucun SSID affiché. BLE jamais initialisé.
- Toute extension passe par une décision explicite du product owner.

## Règle v1 (identique à daoa-ios)

Daoa v1 (dépôt privé, chemin dans `$DAOA_V1_PATH`) est une
**spécification de comportement, jamais une source de code**. On reproduit la
mécanique observée (documentée dans `docs/03_MECANIQUE_TIRAGE.md`), on ne
porte ni code, ni structure. Le contenu (textes des 64 figures) en est
extrait par script — c'est du contenu Daoa original, pas du code.

## Décisions techniques actées

- **Stack** : PlatformIO + Arduino-ESP32 (core 3.x via pioarduino),
  `M5Cardputer` ≥ 1.1.1, `M5Unified`, `M5GFX` (LovyanGFX). Pas de LVGL.
- **Probabilités du tirage** : méthode traditionnelle des trois pièces
  (6 → 1/8, 7 → 3/8, 8 → 3/8, 9 → 1/8). Ratifié 2026-08-08 — écart
  volontaire par rapport à v1 (uniforme).
- **Aléa** : `esp_random()` (TRNG matériel). Le tirage est réel ; aucune
  entrée utilisateur ne l'influence. **Amendement 2026-08-09 — lecture du
  lieu** : l'environnement radio ambiant peut pondérer le tirage de façon
  **bornée** (biais de pièce ≤ ±0,05, vieillissement ≤ 0,04 — voir
  `docs/03`), **documentée** dans l'aide, et **désactivable** (réglages,
  touche `s`). Activée par défaut : c'est la fonction différenciatrice de
  l'objet. L'utilisateur ne choisit toujours rien ; l'honnêteté de la
  formule est la condition de la règle.
- **Contenu compilé** : les textes sont générés en en-têtes C++ par
  `scripts/extract_content.py`. Pas de JSON à l'exécution.
- **Simulateur SDL** : le rendu doit rester exécutable sur Mac (env `sim`)
  pour itérer visuellement sans flasher. Le code UI ne doit donc dépendre
  que de l'API LovyanGFX (`lgfx::`), jamais du matériel directement.
- La logique de tirage (`src/core/`) est du C++ pur, sans dépendance
  Arduino/M5, testée par `pio test` en natif.

## Frontière public/privé (ferme — décidée le 2026-08-09)

Le dépôt GitHub est **public** : il contient le moteur (logique de tirage,
composants de rendu, tokens, simulateur, tests, docs), **jamais la marque
ni le contenu Daoa**. Sont exclus du versionnage et purgés de l'historique :

- `design/` (handoff, retours) et `.claude/skills/daoa-mini-design/`
  (le design system livré, avec les assets de marque) ;
- les en-têtes **générés** qui encodent marque ou contenu :
  `src/content/hexagrams_data.h`, `src/content/hexagram_details_data.h`
  (corpus propriétaire bilingue) et `src/ui/assets/` (lettermark, sceau,
  masques, fond promo). `src/core/kingwen_table.h` reste public
  (patrimoine mathématique, pas du contenu Daoa).

Conséquence assumée : un clone public **ne compile pas** sans les sources
privées. `scripts/extract_content.py` et `scripts/convert_assets.py` les
localisent par `$DAOA_V1_PATH` et `$DAOA_IOS_PATH` — **aucun chemin de
machine n'est écrit dans le dépôt**. Sur ce poste : `source private/env.sh`
puis relancer les deux scripts. Avant tout commit
d'un nouveau fichier généré, se demander : encode-t-il de la marque ou du
contenu ? Si oui, il rejoint le `.gitignore`.

**Nuance des captures d'écran** (2026-08-09) : `docs/images/` est
versionné. Une capture 240 × 135 du firmware en marche est du matériel
**promotionnel** — c'est la raison d'être de l'objet — et ne redonne ni
les assets sources (masques, vectoriels) ni le corpus exploitable. Ce qui
reste interdit : les fichiers sources de marque et les en-têtes générés.

## Standard visuel

Le même que Daoa 2 : calme, précis, premium, minimal, typographique,
tactile. Jamais : fantasy, cliché ésotérique/tarot, casino, enfantin,
néon mystique. Sur 240 × 135, cela se traduit par : peu d'éléments, des
fontes anti-aliasées, des animations lentes et physiques (easing), du
contraste maîtrisé (palette encre/papier), aucun chrome décoratif.

L'UX doit exploiter à fond ce que le matériel offre : sprites et
anti-aliasing M5GFX, animations d'apparition des traits, retour sonore
discret (codec ES8311), éventuellement geste IMU. L'ambition visuelle est
haute précisément parce que l'écran est petit.

## Rôles

Pierre est product owner, directeur visuel, testeur et décideur final.
Claude Code est architecte technique, implémenteur, opérateur du
simulateur et mainteneur de la documentation. Travailler en petites étapes
validées visuellement (simulateur d'abord, appareil ensuite).

## Index documentation

- `docs/00_ANALYSE_BESOINS.md` — analyse initiale (matériel, libs, outils).
- `docs/01_DECISIONS.md` — journal des décisions actées.
- `docs/02_ARCHITECTURE.md` — arborescence, environnements de build,
  pipeline de génération du contenu.
- `docs/03_MECANIQUE_TIRAGE.md` — spécification du tirage (source de
  vérité de la mécanique).
- `docs/04_UX_RENDU.md` — écrans, direction visuelle, fontes, animations.
- `docs/05_ROADMAP.md` — phases et état courant.

`private/` (hors dépôt) : stratégie de lancement, analyse du catalogue
M5Burner, textes de publication. Rien de ce qui relève du positionnement
ou de la tactique ne doit revenir dans `docs/`.
