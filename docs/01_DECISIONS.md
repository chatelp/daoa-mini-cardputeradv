# Journal des décisions

Format : date — décision — statut. Les décisions actées ne se rediscutent
qu'avec le product owner.

## 2026-08-08 — Périmètre produit — ACTÉ

Tirage ligne à ligne uniquement (comme Daoa v1), sans question en entrée,
sans journal, sans IA/backend, sans réseau. Sortie : figure + texte court +
figure complémentaire, puis écran promo (QR vers l'app complète Daoa).
Positionnement : outil promotionnel de niche.

## 2026-08-08 — Probabilités du tirage — ACTÉ

Méthode traditionnelle des trois pièces : 6 → 1/8, 7 → 3/8, 8 → 3/8,
9 → 1/8. Écart volontaire par rapport à v1 (uniforme 25 %) : plus juste
symboliquement, et les lignes mutables redeviennent des événements rares.

## 2026-08-08 — Daoa v1 = spec, pas source — ACTÉ

Réécriture C++ propre. Aucun port de code ni de structure depuis
`yi-jing-chatelp`. Seul le **contenu** (textes des figures) est extrait par
script. Même discipline que daoa-ios.

## 2026-08-08 — Stack technique — ACTÉ

PlatformIO + Arduino-ESP32 core 3.x (pioarduino), M5Cardputer ≥ 1.1.1,
M5Unified, M5GFX. Pas de LVGL (surdimensionné pour 4 écrans typographiques,
et il imposerait son look ; M5GFX/LovyanGFX donne sprites, fontes
anti-aliasées et contrôle pixel total). Pas d'ArduinoJson (contenu compilé).
Simulateur SDL (LovyanGFX) pour itérer le rendu sur Mac.

## 2026-08-08 — Langues — ACTÉ

FR et EN embarqués tous les deux, bascule par touche. Textes = contenu Daoa
original extrait de v1 (60 premiers caractères : nom + description courte).

## 2026-08-09 — Lecture du lieu (scan WiFi pondérant le tirage) — ACTÉ

Le Cardputer scanne passivement le WiFi ambiant avant le tirage et en
dérive deux lectures : la **densité du champ** (réseaux + énergie des
signaux) et son **agitation** (dispersion des canaux, variance). Elles
pondèrent les trois pièces de façon bornée : biais de pièce β ≤ ±0,05
(yin/yang), vieillissement γ ≤ 0,04 (mutabilité). Le TRNG reste la source ;
aux plafonds, P(vieux yin) reste dans [9 %, 16 %]. Jamais de connexion,
radio coupée après le scan, rien de stocké ni transmis, aucun SSID montré.
**Activée par défaut** (fonction différenciatrice de l'objet), coupable
dans les réglages (touche `s`). Formule documentée dans docs/03 et
résumée dans l'aide. Interface : champ de particules vivant pendant le
tirage — graphique, jamais un graphe. Amende les deux verrous historiques
(hors-ligne strict ; tirage sans influence) — CLAUDE.md à jour.
