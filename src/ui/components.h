// Composants de rendu du design system daoa-mini-design, en LovyanGFX.
// Specs : .claude/skills/daoa-mini-design/components/*/{*.d.ts,*.prompt.md}.
// Aucune cote en dur : tout vient de daoa_tokens.h. Les couleurs sont
// passées par l'appelant (déjà passées par son fondu d'écran) ; les
// composants n'imposent que la géométrie et les fontes.
#pragma once

#include <cstdint>

#include "../content/content.h"
#include "assets/ds_assets.h"
#include "daoa_key_labels.h"
#include "daoa_tokens.h"
#include "gfx.h"
#include "screen_footers.h"

namespace daoa::ui {

using Sprite = lgfx::LGFX_Sprite;

// ---- structures ---------------------------------------------------------

// Un segment d'invite : key est rendu en papier, text en pierre.
// key et text sont des identifiants de daoa_key_labels.h (ou des littéraux,
// qui passent tels quels — touches sérigraphiées, textes dynamiques).
struct PromptSeg {
    const char* key;
    const char* text;
};

// Une ligne du pavé de lecture, déjà coupée (le composant ne rewrappe pas).
// meta = intertitre de section, rendu en pierre.
struct ReaderLine {
    const char* text;
    uint16_t len;
    bool meta;
};

// Taille de trait yin/yang (YAO_SIZES du design system).
struct YaoSize {
    int w, t, gap, split, mark;
};

inline constexpr YaoSize YAO_L{tokens::YAO_L_W, tokens::YAO_L_T, tokens::YAO_L_GAP,
                               tokens::YAO_L_SPLIT, tokens::MARK_LG};
inline constexpr YaoSize YAO_M{tokens::YAO_M_W, tokens::YAO_M_T, tokens::YAO_M_GAP,
                               tokens::YAO_M_SPLIT, tokens::MARK_SM};
inline constexpr YaoSize YAO_S{tokens::YAO_S_W, tokens::YAO_S_T, tokens::YAO_S_GAP,
                               tokens::YAO_S_SPLIT, tokens::MARK_SM};

// Un trait d'une figure, donné du bas vers le haut comme on tire.
struct HexLine {
    bool yang;
    bool mutating;
};

constexpr int hexagramHeight(const YaoSize& s) { return 6 * s.t + 5 * s.gap; }

// Jeu de couleurs d'un écran, après fondu d'entrée.
struct Palette {
    uint16_t bg;        // MINI_BG
    uint16_t text;      // MINI_TEXT (papier)
    uint16_t meta;      // MINI_TEXT_META (pierre)
    uint16_t mutation;  // MINI_MUTATION (cinabre)
    uint16_t brand;     // MINI_BRAND (terracotta)
};

// ---- primitives ---------------------------------------------------------

// Blit d'un masque monochrome 8 bits teinté (alpha = couverture), fondu
// vers bg. `strength` module l'opacité globale (animations).
void drawMask(Sprite& s, const MaskAsset& m, int x, int y, uint16_t color, uint16_t bg,
              float strength = 1.f);

// ---- chrome -------------------------------------------------------------

// Bande supérieure de 18 px : contexte à gauche (tronqué, jamais deux
// lignes), pagination à droite (pastilles ≤ DOT_MAX pages, chiffres au-delà,
// rien sous 2 pages). Toujours en pierre.
void drawDocHeader(Sprite& s, const Palette& p, const char* context, int page = 1,
                   int pages = 1);

// Bande inférieure de 18 px : légende en pierre à gauche, raccourcis à
// droite (touche papier, libellé pierre, séparés par « · »). La barre est
// décrite par une FooterSpec (identifiants) et résolue ici selon la langue
// — les écrans ne portent aucune chaîne en dur (HANDOFF-v2, tâche 2).
void drawFooterBar(Sprite& s, const Palette& p, Lang lang, const FooterSpec& spec);

// Pastilles de page : carrés de DOT px espacés de DOT_GAP, jamais des ronds.
// xRight = bord droit de l'emprise ; cy = centre vertical.
void drawPageDots(Sprite& s, const Palette& p, int xRight, int cy, int count, int index);

// Invite centrée dans la bande de contenu. Une seule invite par écran.
// Les segments sont résolus par keyLabel() selon la langue.
void drawPrompt(Sprite& s, const Palette& p, Lang lang, int y, const PromptSeg* segs,
                int count);

// Mentions des coins hauts de l'accueil (≤ 16 caractères chacune, pierre).
void drawCornerHints(Sprite& s, const Palette& p, const char* left, const char* right);

// ---- figure -------------------------------------------------------------

// Le trait unitaire. reveal01 anime la pose (croissance depuis le centre).
void drawYaoLine(Sprite& s, int x, int y, bool yang, const YaoSize& size, uint16_t color,
                 float reveal01 = 1.f);

// Figure complète, traits donnés du bas vers le haut. Les traits non tirés
// (i ≥ revealed) sont des carrés en pierre : la hauteur ne change jamais.
// lastReveal01 anime le dernier trait tiré ; les marques de mutation
// (masques cinabre) apparaissent en fin de pose.
void drawHexagram(Sprite& s, const Palette& p, int x, int yTop, const HexLine* lines,
                  int revealed, const YaoSize& size, bool showMutations = true,
                  float lastReveal01 = 1.f);

// ---- texte --------------------------------------------------------------

// Pavé de lecture : MINI_READER_LINES lignes maximum, déjà coupées.
void drawReader(Sprite& s, const Palette& p, const ReaderLine* lines, int count,
                int top = tokens::MINI_CONTENT_Y);

// Identité de la figure : sinogramme (display 24, fonte passée par
// l'appelant — efontCN ou repli efontJA), nom (titre 16, repli corps 12 au
// delà de MINI_TITLE_COLS), glose 2 lignes maximum.
void drawFigureTitle(Sprite& s, const Palette& p, const char* glyph,
                     const lgfx::IFont* glyphFont, const char* name, const char* gloss1,
                     const char* gloss2, int top);

// ---- panneaux -----------------------------------------------------------

// Emprise papier du QR promo : panneau QR_SIZE × QR_SIZE, rayon R2, quiet
// zone QR_QUIET. Le code lui-même est dessiné par l'appelant dans la zone
// renvoyée (carrée, alignée sur `moduleCount` modules entiers).
struct QrArea {
    int x, y, modulePx;
};
QrArea drawQrPanel(Sprite& s, int x, int y, int moduleCount);

}  // namespace daoa::ui
