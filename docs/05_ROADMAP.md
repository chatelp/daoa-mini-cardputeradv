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

## Phase 1.2 — Design system — FAIT (2026-08-09)

- [x] Handoff Claude Design (design/claude-design-handoff/) → livrable reçu
      et installé en skill projet (.claude/skills/daoa-mini-design)
- [x] Tokens traduits en constexpr (src/ui/daoa_tokens.h) : bandes 18/99/18,
      espacements base 2, type 12/16/24, yao L/M/S (1:12), palette sémantique
- [x] Composants LovyanGFX (src/ui/components.{h,cpp}) : DocHeader,
      FooterBar (touche papier/libellé pierre), PageDots, Prompt, CornerHints,
      YaoLine, Hexagram (traits en attente, marques en masques cinabre),
      Reader, FigureTitle, QrPanel
- [x] Assets bitmap intégrés : mutations 16/12/8, ornement accueil,
      sceau 36×46, fond promo 565, lettermark 104 px
- [x] Six écrans recomposés sur les gabarits ; validé simulateur + appareil
- [ ] Reste du système non intégré : trigrammes 24×24 (aucun usage à date)

## Phase 1.5 — Rituel et finitions

- [ ] Direction typographique définitive (fontes VLW dédiées)
- [ ] Sous-ensemble CJK 64 glyphes grand format
- [ ] Animation de révélation peaufinée, transitions entre écrans
- [ ] Son discret (ES8311), volume faible par défaut
- [ ] Light-sleep / extinction écran en veille
- [ ] Option : geste IMU (secouer) pour tirer une ligne

## Phase 2 — Lecture du lieu + sons — FAIT (2026-08-09)

- [x] Doctrine amendée (hors-ligne strict → scan passif borné ; influence
      documentée/désactivable, activée par défaut)
- [x] core/ambient : densité/agitation → β ≤ ±0,05, γ ≤ 0,04, construction
      deux-Bernoulli équivalente aux trois pièces, 5 tests
- [x] Scan WiFi réel (radio coupée après), hôte simulé côté Mac
- [x] Champ de particules vivant + rituel « lecture du lieu… »
- [x] Vue technique (w) : réseaux, valeurs, β/γ, probabilités
- [x] Réglages (s) : lecture du lieu + sons, persistants (NVS)
- [x] Sons PCM doux (pentatonique des traits, mutations, résolution, ticks)
- [x] Aide enrichie (section lecture du lieu + touches s/w)
- [x] Partitions max_app_8MB (pile WiFi) — flash 41 % de 8 Mo
- [x] Validation sur appareil : scan réel, champ, sons — OK (2026-08-09)
- [x] Champ enrichi : halos, ondes concentriques, réponse au tirage
- [x] Bascules (w/h/s) sans perte du tirage en cours
- [x] Indications de touches sur accueil (s) et tirage (w)
- [ ] Handoff v3 à Claude Design : langage visuel du champ + écran réglages

## Phase 2.1 — Vitrine publique — FAIT (2026-08-09)

- [x] README anglais (public maker international) + README.fr.md
- [x] Captures versionnées dans `docs/images/` (nuance de doctrine :
      une capture du firmware est promotionnelle, pas un asset source)
- [x] GIF du champ vivant, mode `--frames` du simulateur (60 frames)
- [x] Bandeau composite quatre écrans

## Reste à faire

- [ ] Typographie : fonte VLW dédiée (latin soigné + 64 glyphes exacts)
- [ ] Light-sleep / extinction écran en veille
- [ ] Option : geste IMU (secouer) pour tirer une ligne
- [ ] Licence du dépôt public à décider
- [ ] Réinstaller M5Launcher (M5Burner) à la fin des itérations série
