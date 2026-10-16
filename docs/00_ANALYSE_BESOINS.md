# Daoa Mini — Analyse des besoins (Cardputer ADV)

Date : 2026-08-08. Périmètre décidé par le product owner :

- **Tirage ligne à ligne uniquement**, comme Daoa v1 (`yi-jing-chatelp`) — pas de tirage par cartes.
- **Pas de question en entrée**, pas de journal, pas d'IA/backend, pas de crédits.
- Sortie : la figure tirée + son texte (+ figure complémentaire, comme v1).
- Positionnement : **petit outil promotionnel de niche** pour la version complète de Daoa (iPhone/Android).

## 1. Mécanique de tirage (référence : Daoa v1)

**Règle (même discipline que daoa-ios) : v1 est une spécification de
comportement, pas une source de code.** On ne porte ni le code, ni la
structure de `hexagramCalculator.ts` / `useYiJingDrawing.ts` — on réécrit
proprement en C++ le comportement observé, décrit ci-dessous. Exemples de
nettoyages attendus : la table binaire→King Wen devient une donnée générée
(pas un objet littéral recopié à la main, et pas de reverse-mapping
reconstruit à chaque appel comme le fait `getHexagramLines` de v1) ; le
tirage est une petite machine à états sans les artifices React
(`setTimeout`, garde anti-spam) — le délai rituel devient une animation
pilotée par le rendu.

Comportement à reproduire :

1. 6 lignes tirées une à une, de bas en haut. Chaque ligne vaut 6, 7, 8 ou 9
   (6 = yin mutable, 7 = yang fixe, 8 = yin fixe, 9 = yang mutable).
2. Figure originale : lignes → binaire → numéro King Wen via la table de
   correspondance (ordre Javary/Faure) — table à porter telle quelle depuis v1.
3. Figure complémentaire : 6→7, 9→8, lignes fixes inchangées ; même table.
4. Affichage : figure originale (lignes mutables marquées), nom + texte,
   puis figure complémentaire.

**Décision produit à trancher** : v1 tire chaque valeur avec une probabilité
uniforme (25 % chacune). La méthode traditionnelle des trois pièces donne
6 → 1/8, 7 → 3/8, 8 → 3/8, 9 → 1/8. Recommandation : adopter les probabilités
traditionnelles (plus juste symboliquement, mutations plus rares donc plus
marquantes) — mais c'est un écart volontaire par rapport à v1, à valider.

**Source d'aléa** : `esp_random()` (TRNG matériel de l'ESP32-S3). Le tirage est
réel — règle Daoa conservée.

## 2. Contenu

- `shared/data/hexagrams.ts` de v1 (~20 Ko) : 64 figures, nom (caractère +
  pinyin + titre) et description courte, FR/EN. **Suffisant pour le MVP** — le
  positionnement promo appelle un texte court qui donne envie d'aller vers
  l'app complète, pas une lecture longue.
- Option d'enrichissement : descriptions plus développées du corpus daoa-ios
  (`HexagramGeneral.swift`), si le texte court paraît trop maigre à l'usage.
- Écran final promo : « Découvrez Daoa sur iPhone » + **QR code affiché à
  l'écran** vers la page App Store / site. 240×135 suffit pour un QR version 2-3
  (25×33 modules) scannable.

Outillage : script Python de conversion `hexagrams.ts` → en-têtes C++ constants
générés à la compilation (pas de JSON ni d'ArduinoJson à l'exécution).

## 3. Matériel (contraintes)

| Élément | Spec | Conséquence |
|---|---|---|
| MCU | ESP32-S3FN8 (StampS3A), dual-core 240 MHz, 8 Mo flash, pas de PSRAM, 512 Ko SRAM | Très large pour ce périmètre |
| Écran | ST7789V2, 1,14", 240 × 135 px | Framebuffer 16 bits plein écran = 65 Ko → OK en SRAM ; textes → écrans successifs |
| Clavier | 56 touches (contrôleur I²C TCA8418) | **Navigation seulement** (aucune saisie de texte) : une touche = lancer une ligne, flèches = pages. Le problème des accents disparaît avec la question |
| IMU | BMI270 | Option rituelle : secouer pour lancer une ligne |
| Audio | ES8311 + HP 1 W | Option : son discret par ligne révélée |
| RNG | TRNG matériel | Tirage réel |
| Batterie | 1750 mAh + interrupteur | Session courte ; light-sleep en veille suffit |

> Révision matérielle du 2026-07-13 : la batterie passe de 1750 à 1500 mAh
> (journal de versions de la fiche officielle). Le raisonnement ci-dessus
> n'en est pas affecté — la consommation annoncée est de 120 mA au repos,
> 132 mA en WiFi sous 4,2 V.

## 4. Chaîne d'outils

**Recommandé : VS Code + PlatformIO, framework Arduino-ESP32 (core 3.x).**
ESP-IDF pur (trop lourd pour le gain) et UIFlow2/MicroPython (rendu premium
difficile) écartés.

Outils annexes :
- Script Python d'extraction du contenu (§2).
- Convertisseur de fontes (`lv_font_conv` ou VLW LovyanGFX) : fontes
  anti-aliasées avec accents FR (affichage seulement) + sous-ensemble des
  64 caractères chinois en grande taille.
- **Simulateur SDL de LovyanGFX/M5GFX** : itération du rendu sur Mac sans
  flasher — précieux pour la direction visuelle.
- `esptool` (intégré à PlatformIO) pour le flash.

## 5. Bibliothèques

| Lib | Rôle |
|---|---|
| **M5Cardputer** ≥ 1.1.1 | Support officiel Cardputer-ADV, clavier TCA8418 |
| **M5Unified** | Alim, veille, haut-parleur (dépendance de M5Cardputer) |
| **M5GFX** (LovyanGFX) | Rendu : sprites, fontes UTF-8 anti-aliasées, animations |
| **QRCode** (lib C légère, p. ex. ricmoo/QRCode) | QR promo de l'écran final |

Pas de LVGL (surdimensionné), pas d'ArduinoJson (contenu compilé), pas de
WiFi/HTTPS (aucun réseau dans ce périmètre).

## 6. Écrans (4 seulement)

1. **Accueil** : marque Daoa, « appuyez pour commencer ».
2. **Tirage** : les 6 lignes se révèlent une à une (animation type
   `LineRevealAnimation` de v1, délai 600-800 ms), une pression par ligne.
3. **Résultat** : figure + caractère + pinyin + nom + texte court (FR/EN,
   bascule par touche), lignes mutables marquées, puis figure complémentaire.
4. **Promo** : invitation + QR code vers l'app complète.

## 7. Points durs

1. **Rendu typographique premium sur 240 × 135** — le vrai sujet du projet.
   Fontes VLW anti-aliasées, hiérarchie stricte, textes courts.
2. **Glyphes chinois** : sous-ensemble de 64 caractères, pas de fonte CJK
   complète.
3. **Probabilités du tirage** (§1) : décision produit à acter avant le code.
4. Langue : FR/EN embarqués tous les deux (bascule simple) — cohérent avec le
   bilinguisme Daoa.

## 8. Découpage proposé

- **Phase 0 — socle** : projet PlatformIO, écran + clavier OK, fonte FR
  accentuée affichée, simulateur SDL opérationnel.
- **Phase 1 — MVP** : tirage complet ligne à ligne (probabilités actées),
  figure + complémentaire + textes, écran promo QR. C'est tout le produit.
- **Phase 1.5 — rituel** : animation de révélation soignée, geste IMU
  optionnel, son discret, light-sleep.
