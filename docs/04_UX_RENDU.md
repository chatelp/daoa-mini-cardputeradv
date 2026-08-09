# UX et rendu

Standard : celui de Daoa 2 — calme, précis, premium, typographique. L'écran
fait 240 × 135 : chaque pixel compte, l'ambition passe par la sobriété, la
typo anti-aliasée et le mouvement, pas par la densité.

## Écrans

1. **Accueil** — marque « Daoa » + sous-titre, respiration lente (léger
   pulse d'opacité), « appuyer pour commencer ».
2. **Tirage** — les 6 traits se révèlent de bas en haut, un par pression.
   Chaque révélation : montée en fondu + easing, marque de mutation (point /
   croix discrète) pour 6 et 9. Micro-son optionnel par le haut-parleur.
3. **Résultat** — caractère chinois en grand, pinyin, nom, description
   courte ; lignes mutables signalées ; accès à la figure complémentaire
   (pagination). Bascule FR/EN par touche.
4. **Promo** — « Daoa, l'app complète sur iPhone » + QR code (petite lib C
   type ricmoo/QRCode ; QR version 2-3 lisible sur 135 px de haut).

## Techniques de rendu (tout M5GFX/LovyanGFX)

- **Sprites plein écran** (240 × 135 × 16 bits = 65 Ko en SRAM) : tout se
  dessine hors écran puis se pousse en un bloc — zéro scintillement.
- **Fontes** (état vérifié au `--fonttest` du simulateur) : les DejaVu de
  lgfx sont ASCII pur (pas d'accents) — réservées au wordmark « Daoa ».
  Texte courant FR/EN : **efontJA** (latin accentué complet). Hanzi :
  **efontCN_24_b**, avec bascule efontJA_24_b pour 遯 (33), 夬 (43) et
  姤 (44), absents d'efontCN. Cible phase 1.5 : fontes VLW dédiées
  anti-aliasées (latin soigné + sous-ensemble exact des 64 glyphes),
  jamais une CJK complète.
- **Animations** : easing maison (`anim/easing.h` — cubic/quint out,
  pulse), pilotées par le temps (`millis`), jamais par des délais bloquants.
  Le simulateur SDL permet de régler les courbes à l'œil sur Mac.
- **Palette** : interface sombre (décision appareil 2026-08-08) — fond
  noir pur qui se fond avec la lunette de l'écran, texte papier chaud,
  gris pierre pour le secondaire, cinabre éclairci pour les marques de
  mutation. Définie dans `ui/theme.h`. Taille de fonte minimale : 12 px
  (efont 10 illisible sur l'écran physique).

## Entrées

| Action | Appareil (clavier) | Simulateur (SDL) |
|---|---|---|
| Avancer / tirer une ligne | Espace ou Entrée | Espace / Entrée |
| Page précédente / suivante | `,` / `/` | ← / → |
| Bascule FR/EN | `l` | `l` |
| Recommencer | `` ` `` (Esc) | Échap |

Pas de saisie de texte nulle part.

## Boucle de validation visuelle

1. Itérer dans le simulateur (`pio run -e sim`, capture `--shot`).
2. Revue par le product owner (captures ou fenêtre SDL).
3. Flash sur l'appareil pour valider couleurs réelles et lisibilité
   (l'écran physique rend différemment du sim, surtout le contraste).

## Lecture du lieu, réglages, sons (2026-08-09)

- **Rituel** : à l'entrée du tirage, ~1,4 s de « lecture du lieu… »
  (le temps réel du scan devient le rituel), puis le champ se révèle.
- **Champ ambiant** : une particule 2×2 par réseau perçu, position liée
  au canal, luminance à l'énergie du signal, dérive lente et respiration
  déphasées (nombre d'or) — vivant, jamais un graphe. En pierre, derrière
  la figure.
- **Vue technique** (touche `w`, depuis tirage ou résultat) : réseaux,
  canaux, dBm, densité/agitation, β/γ, probabilités résultantes — la
  transparence est la condition de la règle. Public geek assumé.
- **Réglages** (touche `s`) : trois lignes (lecture du lieu, sons,
  volume), ↑↓ sélection, espace bascule ou cycle le niveau, persistant
  (NVS). Le volume a trois niveaux nommés (doux · moyen · fort, soit
  70/150/235), **doux par défaut** — l'objet se tient en main, dans le
  calme et s'éteint visuellement quand les sons sont coupés ; le
  tick de confirmation sort au nouveau niveau, pour l'entendre aussitôt.
- **Sons** : sinusoïdes brèves à décroissance douce (PCM, jamais tone()) —
  les six traits égrènent une pentatonique montante (A3→A4), doublage
  grave sur les mutables, résolution E4→A3 au résultat, tick discret sur
  les pages, carillon en fin de lecture du lieu. Coupables, volume réglable.
  **Registre imposé par le matériel** : le haut-parleur de 1 W ne restitue
  rien sous ~400 Hz — la gamme est donc écrite en A5–A6 (880–1760 Hz), et
  non dans l'octave où elle se lirait naturellement. Constaté le
  2026-08-09 : en A3–A4, le tirage était muet alors que les ticks aigus
  s'entendaient.
