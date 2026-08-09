# Architecture

## Arborescence

```
platformio.ini          # 3 environnements : cardputer-adv, sim, test natif
scripts/
  extract_content.py    # v1 hexagrams.ts + table King Wen → en-têtes C++
src/
  core/                 # logique pure (C++17, zéro dépendance Arduino/M5)
    casting.h/.cpp      # machine à états du tirage, trois pièces, King Wen
  content/
    hexagrams_data.h    # GÉNÉRÉ — ne pas éditer à la main
    content.h           # accès typé au contenu (langue, figure)
  anim/
    easing.h            # easing + petit animateur temporel
  ui/
    gfx.h               # point d'inclusion unique LovyanGFX/M5GFX
    theme.h             # palette encre/papier, échelles typographiques
    app.h/.cpp          # machine à états des écrans + rendu
  main.cpp              # entrée appareil (M5Cardputer) — exclu du sim
  sim/
    main_sdl.cpp        # entrée simulateur SDL — exclu de l'appareil
test/
  test_casting/         # pio test -e test-native
docs/
```

## Environnements de build

- `cardputer-adv` — firmware. Arduino-ESP32 3.x (pioarduino),
  `M5Cardputer` (inclut M5Unified + M5GFX). `pio run -e cardputer-adv`.
- `sim` — binaire macOS, rendu via LovyanGFX `Panel_sdl` (SDL2 Homebrew),
  fenêtre 240 × 135 zoomée. Mode `--shot chemin.bmp` pour capturer l'écran
  (vérification visuelle par l'agent). `pio run -e sim` puis exécution du
  binaire `.pio/build/sim/program`.
- `test-native` — tests unitaires de `src/core/` (Unity). `pio test -e test-native`.

## Règles de dépendance

- `core/` ne dépend de rien (ni Arduino, ni M5, ni lgfx). L'aléa est
  **injecté** (pointeur de fonction `uint32_t rand32()`), branché sur
  `esp_random` côté appareil, sur un PRNG déterministe côté tests/sim.
- `ui/` dépend de `core/`, `content/`, `anim/` et de l'API `lgfx::`
  uniquement — jamais de `M5Cardputer` directement. Les entrées sont
  abstraites en `daoa::Key` ; `main.cpp` (clavier TCA8418) et
  `main_sdl.cpp` (clavier SDL) font chacun le mapping.
- `content/hexagrams_data.h` est généré ; toute correction de texte se fait
  dans la source v1 ou dans le script, jamais dans le fichier généré.
- **Frontière public/privé** (CLAUDE.md) : les en-têtes générés portant le
  contenu ou la marque (`content/hexagrams_data.h`,
  `content/hexagram_details_data.h`, `ui/assets/`) ne sont **pas
  versionnés** — présents sur disque, régénérables par les scripts, exclus
  du dépôt public. Un clone sans les sources privées ne compile pas.

## Pipeline de contenu

`scripts/extract_content.py` lit dans le dépôt v1 (chemin par défaut
`$DAOA_V1_PATH`) :

1. `shared/data/hexagrams.ts` — 64 × {number, name, description, nameEn,
   descriptionEn}. Le champ `name` (« 乾 Qián - Le Créatif ») est décomposé
   en caractère(s), pinyin, titre.
2. La table `BINARY_TO_HEXAGRAM` de `shared/logic/hexagramCalculator.ts`
   (chaînes haut→bas), convertie en table indexée par bits bas→haut.

Sortie : `src/content/hexagrams_data.h` (constantes, UTF-8) + garde-fous
(64 entrées, bijection King Wen). Relancer : `python3 scripts/extract_content.py`.
