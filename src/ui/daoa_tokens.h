// Tokens du design system Daoa Mini — traduction C++ de
// .claude/skills/daoa-mini-design/tokens/{colors,spacing,geometry,typography}.css
// Une constexpr par token, mêmes noms qu'en CSS, en majuscules.
// Ne rien coter en dur dans les composants : tout vient d'ici.
#pragma once

#include <cstdint>

namespace daoa::tokens {

constexpr uint16_t rgb565(uint32_t rgb888) {
    const uint32_t r = (rgb888 >> 16) & 0xFF;
    const uint32_t g = (rgb888 >> 8) & 0xFF;
    const uint32_t b = rgb888 & 0xFF;
    return static_cast<uint16_t>(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

// ---- colors.css — palette imposée (source 24 bits) ---------------------
inline constexpr uint32_t DAOA_BLACK = 0x000000;
inline constexpr uint32_t DAOA_PAPER = 0xECE4D2;
inline constexpr uint32_t DAOA_STONE = 0x948B7A;
inline constexpr uint32_t DAOA_CINNABAR = 0xC65B42;
inline constexpr uint32_t DAOA_TERRACOTTA = 0xC97E58;

// Alias sémantiques : le seul vocabulaire à employer dans un composant.
// (En RGB565 : la quantification EST le rendu réel sur l'appareil.)
inline constexpr uint16_t MINI_BG = rgb565(DAOA_BLACK);
inline constexpr uint16_t MINI_TEXT = rgb565(DAOA_PAPER);
inline constexpr uint16_t MINI_TEXT_META = rgb565(DAOA_STONE);
inline constexpr uint16_t MINI_FIGURE = rgb565(DAOA_PAPER);
inline constexpr uint16_t MINI_FIGURE_PENDING = rgb565(DAOA_STONE);
inline constexpr uint16_t MINI_MUTATION = rgb565(DAOA_CINNABAR);
inline constexpr uint16_t MINI_BRAND = rgb565(DAOA_TERRACOTTA);
inline constexpr uint16_t MINI_PANEL = rgb565(0xFFFFFF);  // panneau QR : blanc scannable
inline constexpr uint16_t MINI_PANEL_INK = rgb565(DAOA_BLACK);

// ---- spacing.css — base 2, sept valeurs, pas une de plus ---------------
inline constexpr int SP_2 = 2;
inline constexpr int SP_4 = 4;
inline constexpr int SP_6 = 6;
inline constexpr int SP_8 = 8;
inline constexpr int SP_12 = 12;
inline constexpr int SP_16 = 16;
inline constexpr int SP_24 = 24;

// ---- geometry.css — écran, bandes, épaisseurs, rayons ------------------
inline constexpr int MINI_W = 240;
inline constexpr int MINI_H = 135;
inline constexpr int MINI_MARGIN_X = 6;
inline constexpr int MINI_COL_W = 228;  // 38 colonnes à 6 px d'avance

inline constexpr int MINI_HEADER_H = 18;
inline constexpr int MINI_CONTENT_Y = 18;
inline constexpr int MINI_CONTENT_H = 99;  // 7 lignes de 14 px, 1 px de mou
inline constexpr int MINI_FOOTER_Y = 117;
inline constexpr int MINI_FOOTER_H = 18;
inline constexpr int MINI_BAND_PAD_Y = 3;  // (18 − 12) / 2

inline constexpr int W1 = 1;  // contours de pastille, cadre QR — jamais un trait isolé
inline constexpr int W2 = 2;  // épaisseur minimale d'une forme pleine
inline constexpr int W4 = 4;
inline constexpr int W8 = 8;

inline constexpr int R0 = 0;  // défaut : tout, y compris les traits
inline constexpr int R2 = 2;  // panneaux pleins ≥ 24 px (QR) — seule exception

// Traits yin/yang : largeur, épaisseur, écart entre traits, coupure du yin.
// Rapport épaisseur/largeur ~1:12 ; coupure = 2 × épaisseur.
inline constexpr int YAO_L_W = 96, YAO_L_T = 8, YAO_L_GAP = 6, YAO_L_SPLIT = 16;  // figure 96 × 78
inline constexpr int YAO_M_W = 44, YAO_M_T = 4, YAO_M_GAP = 3, YAO_M_SPLIT = 8;   // figure 44 × 39
inline constexpr int YAO_S_W = 24, YAO_S_T = 2, YAO_S_GAP = 2, YAO_S_SPLIT = 4;   // figure 24 × 22

// Marques de mutation : diamètre optique et épaisseur de trait.
inline constexpr int MARK_LG = 16, MARK_LG_W = 3;
inline constexpr int MARK_MD = 12, MARK_MD_W = 2;
inline constexpr int MARK_SM = 8, MARK_SM_W = 2;
inline constexpr int MARK_GUTTER = 8;  // écart bord droit du trait → marque

// Panneau QR de l'écran promo.
inline constexpr int QR_SIZE = 116;
inline constexpr int QR_QUIET = 4;

// Pastilles de page.
inline constexpr int DOT = 2;
inline constexpr int DOT_GAP = 4;
inline constexpr int DOT_MAX = 12;  // au-delà, pagination chiffrée

// ---- typography.css — trois tailles, une famille bitmap ----------------
inline constexpr int MINI_BODY_H = 12, MINI_BODY_ADV = 6, MINI_BODY_LEAD = 14, MINI_BODY_COLS = 38;
inline constexpr int MINI_TITLE_H = 16, MINI_TITLE_ADV = 8, MINI_TITLE_LEAD = 18, MINI_TITLE_COLS = 28;
inline constexpr int MINI_DISPLAY_H = 24, MINI_DISPLAY_ADV = 12, MINI_DISPLAY_LEAD = 26, MINI_DISPLAY_COLS = 19;
inline constexpr int MINI_READER_LINES = 7;

}  // namespace daoa::tokens
