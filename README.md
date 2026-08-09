# Daoa Mini — Cardputer ADV

Tirage Yi Jing ligne à ligne, hors-ligne, sur M5Stack Cardputer ADV
(ESP32-S3). Petit outil promotionnel pour l'app complète
[Daoa](https://daoa.app) sur iPhone/Android.

Accueil → tirage de 6 lignes (méthode des trois pièces, TRNG matériel) →
figure + texte court FR/EN + figure complémentaire → QR vers l'app.

## Développement

Prérequis : PlatformIO (`brew install platformio`), SDL2
(`brew install sdl2`) pour le simulateur.

```bash
# Régénérer le contenu depuis Daoa v1
python3 scripts/extract_content.py

# Tests de la logique de tirage
pio test -e test-native

# Simulateur macOS (fenêtre SDL 240×135 zoomée)
pio run -e sim && .pio/build/sim/program

# Firmware (Cardputer ADV branché en USB)
pio run -e cardputer-adv -t upload
```

**Note** : ce dépôt public contient le moteur, pas la marque ni le contenu
Daoa — les en-têtes générés (corpus des 64 figures, assets de marque) ne
sont pas versionnés, et un clone ne compile donc pas sans les sources
privées. Voir « Frontière public/privé » dans [CLAUDE.md](CLAUDE.md).

Documentation : commencer par [CLAUDE.md](CLAUDE.md), puis `docs/`.
