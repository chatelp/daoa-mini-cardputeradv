# Roadmap

## Phase 0 — Socle — EN COURS (2026-08-08)

- [x] Analyse des besoins (`docs/00_ANALYSE_BESOINS.md`)
- [x] Documentation projet + CLAUDE.md
- [x] Projet PlatformIO 3 environnements (firmware / sim / tests)
- [x] Extraction du contenu v1 → `hexagrams_data.h` (+ table King Wen)
- [x] Logique de tirage `core/` + tests natifs verts (8/8, dont
      distribution trois pièces et bijection de la table)
- [x] Squelette d'app 3 écrans (accueil → tirage → résultat) rendu au
      simulateur SDL, accents FR vérifiés visuellement (captures `--shot`)
- [x] Compilation firmware `cardputer-adv` verte — 2,86 Mo / 3,34 Mo de
      partition app (85 %, dont ~2 Mo de fontes efont : la fonte dédiée de
      la phase 1.5 fera fondre ce chiffre), RAM 8,4 %
- [ ] Flash et validation sur l'appareil (aucun Cardputer branché en USB
      ce jour — `pio run -e cardputer-adv -t upload` à la réception)

Constat typographique (2026-08-08, via `--fonttest` du simulateur) :
les DejaVu de lgfx sont ASCII pur ; efontCN n'a pas les majuscules
accentuées et il lui manque 遯 (33), 夬 (43), 姤 (44) ; efontJA couvre tout
le latin accentué et ces trois glyphes, mais pas les formes simplifiées.
Choix d'intérim : efontJA pour tout le texte latin, efontCN_24_b pour les
hanzi avec bascule efontJA_24_b pour les figures 33/43/44. La fonte dédiée
(64 glyphes + latin soigné, format VLW) reste l'objectif de la phase 1.5.

## Phase 1 — MVP — QUASI COMPLET (2026-08-08)

- [x] Rendu résultat complet (figure complémentaire, mutations, FR/EN)
- [x] Écran promo + QR code vers https://daoa.app (ricmoo/QRCode, v2 ECC M)
- [x] Anglais par défaut ; bascule « L » indiquée sur l'accueil
- [x] Thème sombre (noir pur — retours appareil), fonte minimale 12 px
- [x] Lecture détaillée des figures (corpus Daoa 2 extrait de daoa-ios :
      portrait + Jugement + Image + 6 traits, 64/64 bilingue, ~173 Ko) —
      ↓ depuis le résultat, pagination ↑/↓, esc retour
- [x] Validation sur appareil : boucle complète OK (flash série direct)
- [x] Test de scan réel du QR sur l'écran physique — OK (2026-08-08)
- [x] Validation finale du product owner sur l'ensemble du parcours —
      **MVP validé le 2026-08-08**

## Phase 1.5 — Rituel et finitions

- [ ] Direction typographique définitive (fontes VLW dédiées)
- [ ] Sous-ensemble CJK 64 glyphes grand format
- [ ] Animation de révélation peaufinée, transitions entre écrans
- [ ] Son discret (ES8311), volume faible par défaut
- [ ] Light-sleep / extinction écran en veille
- [ ] Option : geste IMU (secouer) pour tirer une ligne
