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
- **Pas** de journal, pas d'IA, pas de backend, pas de réseau, pas de
  crédits/paiement, pas de comptes.
- Hors-ligne à 100 %. Le WiFi/BLE ne doit même pas être initialisé.
- Toute extension passe par une décision explicite du product owner.

## Règle v1 (identique à daoa-ios)

Daoa v1 (`$DAOA_V1_PATH`) est une
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
  entrée utilisateur ne l'influence.
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
privées (`scripts/extract_content.py` et `scripts/convert_assets.py`
exigent daoa v1, daoa-ios et le skill design). Sur cette machine, tout est
en place — régénérer avec ces deux scripts si besoin. Avant tout commit
d'un nouveau fichier généré, se demander : encode-t-il de la marque ou du
contenu ? Si oui, il rejoint le `.gitignore`.

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
