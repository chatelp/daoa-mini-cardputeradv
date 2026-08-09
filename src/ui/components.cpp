#include "components.h"

#include <cstdio>
#include <cstring>

#include "../anim/easing.h"

namespace daoa::ui {

namespace {

using namespace daoa::tokens;

const lgfx::IFont* bodyFont() { return &fonts::efontJA_12; }
const lgfx::IFont* titleFont() { return &fonts::efontJA_16_b; }

// Ordonnée du texte dans une bande de 18 px (corps de 12).
constexpr int kHeaderTextY = MINI_BAND_PAD_Y;
constexpr int kFooterTextY = MINI_FOOTER_Y + MINI_BAND_PAD_Y;

int textWidthOf(Sprite& s, const lgfx::IFont* f, const char* text) {
    s.setFont(f);
    return s.textWidth(text);
}

// Choisit le masque de mutation pour un diamètre donné.
const MaskAsset& mutationMask(bool yang, int size) {
    if (size >= MARK_LG) return yang ? kMaskMutationYang16 : kMaskMutationYin16;
    if (size >= MARK_MD) return yang ? kMaskMutationYang12 : kMaskMutationYin12;
    return yang ? kMaskMutationYang8 : kMaskMutationYin8;
}

}  // namespace

void drawMask(Sprite& s, const MaskAsset& m, int x, int y, uint16_t color, uint16_t bg,
              float strength) {
    strength = anim::clamp01(strength);
    const uint8_t* p = m.alpha;
    for (int yy = 0; yy < m.height; ++yy) {
        for (int xx = 0; xx < m.width; ++xx, ++p) {
            if (*p == 0) continue;
            const float a = (static_cast<float>(*p) / 255.f) * strength;
            s.drawPixel(x + xx, y + yy, anim::lerp565(bg, color, a));
        }
    }
}

void drawDocHeader(Sprite& s, const Palette& p, const char* context, int page, int pages) {
    s.setFont(bodyFont());
    s.setTextDatum(textdatum_t::top_left);
    s.setTextColor(p.meta, p.bg);

    // Emprise réservée à droite pour la pagination.
    int reserved = 0;
    char pageText[16] = {0};
    const bool paginated = pages >= 2;
    const bool asDots = paginated && pages <= DOT_MAX;
    if (paginated) {
        if (asDots) {
            reserved = pages * DOT + (pages - 1) * DOT_GAP + SP_8;
        } else {
            snprintf(pageText, sizeof(pageText), "%d/%d", page, pages);
            reserved = textWidthOf(s, bodyFont(), pageText) + SP_8;
        }
    }

    // Contexte tronqué à droite : l'en-tête ne fait jamais deux lignes.
    s.setClipRect(MINI_MARGIN_X, 0, MINI_W - 2 * MINI_MARGIN_X - reserved, MINI_HEADER_H);
    s.drawString(context, MINI_MARGIN_X, kHeaderTextY);
    s.clearClipRect();

    if (paginated) {
        if (asDots) {
            drawPageDots(s, p, MINI_W - MINI_MARGIN_X, MINI_HEADER_H / 2, pages, page - 1);
        } else {
            s.setTextDatum(textdatum_t::top_right);
            s.drawString(pageText, MINI_W - MINI_MARGIN_X, kHeaderTextY);
        }
    }
}

void drawFooterBar(Sprite& s, const Palette& p, Lang lang, const FooterSpec& spec) {
    s.setFont(bodyFont());
    s.setTextDatum(textdatum_t::top_left);

    // Raccourcis à droite : touche en papier, libellé en pierre.
    const int count = spec.actionCount > 3 ? 3 : spec.actionCount;
    const char* kSep = " · ";
    const char* keys[3];
    const char* labels[3];
    int total = 0;
    for (int i = 0; i < count; ++i) {
        keys[i] = keyLabel(spec.actions[i].key, lang);
        labels[i] = spec.actions[i].label ? keyLabel(spec.actions[i].label, lang) : nullptr;
        total += textWidthOf(s, bodyFont(), keys[i]);
        if (labels[i] != nullptr) total += MINI_BODY_ADV + textWidthOf(s, bodyFont(), labels[i]);
        if (i + 1 < count) total += textWidthOf(s, bodyFont(), kSep);
    }
    int x = MINI_W - MINI_MARGIN_X - total;
    for (int i = 0; i < count; ++i) {
        s.setTextColor(p.text, p.bg);
        s.drawString(keys[i], x, kFooterTextY);
        x += textWidthOf(s, bodyFont(), keys[i]);
        if (labels[i] != nullptr) {
            x += MINI_BODY_ADV;
            s.setTextColor(p.meta, p.bg);
            s.drawString(labels[i], x, kFooterTextY);
            x += textWidthOf(s, bodyFont(), labels[i]);
        }
        if (i + 1 < count) {
            s.setTextColor(p.meta, p.bg);
            s.drawString(kSep, x, kFooterTextY);
            x += textWidthOf(s, bodyFont(), kSep);
        }
    }

    // Légende à gauche, tronquée avant les raccourcis.
    if (spec.legendId != nullptr) {
        s.setTextColor(p.meta, p.bg);
        s.setClipRect(MINI_MARGIN_X, MINI_FOOTER_Y,
                      MINI_W - 2 * MINI_MARGIN_X - total - SP_12, MINI_FOOTER_H);
        s.drawString(keyLabel(spec.legendId, lang), MINI_MARGIN_X, kFooterTextY);
        s.clearClipRect();
    }
}

void drawPageDots(Sprite& s, const Palette& p, int xRight, int cy, int count, int index) {
    const int width = count * DOT + (count - 1) * DOT_GAP;
    int x = xRight - width;
    for (int i = 0; i < count; ++i) {
        s.fillRect(x, cy - DOT / 2, DOT, DOT, i == index ? p.text : p.meta);
        x += DOT + DOT_GAP;
    }
}

void drawPrompt(Sprite& s, const Palette& p, Lang lang, int y, const PromptSeg* segs,
                int count) {
    s.setFont(bodyFont());
    s.setTextDatum(textdatum_t::top_left);
    const char* kSep = " · ";

    const char* texts[4];
    int total = 0;
    for (int i = 0; i < count && i < 4; ++i) {
        texts[i] = keyLabel(segs[i].key ? segs[i].key : segs[i].text, lang);
        total += textWidthOf(s, bodyFont(), texts[i]);
        if (i + 1 < count) total += textWidthOf(s, bodyFont(), kSep);
    }
    int x = (MINI_W - total) / 2;
    for (int i = 0; i < count && i < 4; ++i) {
        s.setTextColor(segs[i].key ? p.text : p.meta, p.bg);
        s.drawString(texts[i], x, y);
        x += textWidthOf(s, bodyFont(), texts[i]);
        if (i + 1 < count) {
            s.setTextColor(p.meta, p.bg);
            s.drawString(kSep, x, y);
            x += textWidthOf(s, bodyFont(), kSep);
        }
    }
}

void drawCornerHints(Sprite& s, const Palette& p, const char* left, const char* right) {
    s.setFont(bodyFont());
    s.setTextColor(p.meta, p.bg);
    if (left != nullptr) {
        s.setTextDatum(textdatum_t::top_left);
        s.drawString(left, MINI_MARGIN_X, kHeaderTextY);
    }
    if (right != nullptr) {
        s.setTextDatum(textdatum_t::top_right);
        s.drawString(right, MINI_W - MINI_MARGIN_X, kHeaderTextY);
    }
}

void drawYaoLine(Sprite& s, int x, int y, bool yang, const YaoSize& size, uint16_t color,
                 float reveal01) {
    const float e = anim::easeOutQuint(anim::clamp01(reveal01));
    if (yang) {
        const int w = static_cast<int>(size.w * e);
        if (w >= W2) s.fillRect(x + (size.w - w) / 2, y, w, size.t, color);
    } else {
        const int segFull = (size.w - size.split) / 2;
        const int seg = static_cast<int>(segFull * e);
        if (seg >= W2) {
            // Les deux segments croissent du centre vers l'extérieur.
            s.fillRect(x + segFull - seg, y, seg, size.t, color);
            s.fillRect(x + size.w - segFull, y, seg, size.t, color);
        }
    }
}

void drawHexagram(Sprite& s, const Palette& p, int x, int yTop, const HexLine* lines,
                  int revealed, const YaoSize& size, bool showMutations,
                  float lastReveal01) {
    for (int i = 0; i < 6; ++i) {
        // i = 0 → ligne basse, affichée en bas.
        const int y = yTop + (5 - i) * (size.t + size.gap);
        if (i >= revealed) {
            // Trait pas encore tiré : carré en pierre, hauteur constante.
            const int side = size.t;
            s.fillRect(x + size.w / 2 - side / 2, y, side, side, p.meta);
            continue;
        }
        const bool isLast = (i == revealed - 1);
        const float reveal = isLast ? lastReveal01 : 1.f;
        drawYaoLine(s, x, y, lines[i].yang, size, p.text, reveal);

        if (showMutations && lines[i].mutating) {
            // La marque apparaît en fin de pose du trait.
            const float mt = anim::clamp01((reveal - 0.7f) / 0.3f);
            if (mt <= 0.f) continue;
            const MaskAsset& m = mutationMask(lines[i].yang, size.mark);
            drawMask(s, m, x + size.w + MARK_GUTTER, y + size.t / 2 - m.height / 2,
                     p.mutation, p.bg, mt);
        }
    }
}

void drawReader(Sprite& s, const Palette& p, const ReaderLine* lines, int count, int top) {
    if (count > MINI_READER_LINES) count = MINI_READER_LINES;
    s.setFont(bodyFont());
    s.setTextDatum(textdatum_t::top_left);
    char buf[MINI_BODY_COLS * 4 + 4];  // UTF-8
    for (int i = 0; i < count; ++i) {
        snprintf(buf, sizeof(buf), "%.*s", static_cast<int>(lines[i].len), lines[i].text);
        s.setTextColor(lines[i].meta ? p.meta : p.text, p.bg);
        s.drawString(buf, MINI_MARGIN_X, top + i * MINI_BODY_LEAD);
    }
}

void drawFigureTitle(Sprite& s, const Palette& p, const char* glyph,
                     const lgfx::IFont* glyphFont, const char* name, const char* gloss1,
                     const char* gloss2, int top) {
    s.setTextDatum(textdatum_t::top_left);

    // Sinogramme : le seul display de l'écran.
    s.setFont(glyphFont);
    s.setTextColor(p.text, p.bg);
    s.drawString(glyph, MINI_MARGIN_X, top);

    // Nom : titre 16, repli corps 12 au-delà de MINI_TITLE_COLS caractères.
    int nameY = top + MINI_DISPLAY_LEAD;
    s.setFont(titleFont());
    if (s.textWidth(name) > MINI_TITLE_COLS * MINI_TITLE_ADV) s.setFont(bodyFont());
    s.setTextColor(p.text, p.bg);
    s.drawString(name, MINI_MARGIN_X, nameY);

    // Glose : 2 lignes maximum, déjà coupées.
    const int glossY = nameY + MINI_TITLE_LEAD;
    s.setFont(bodyFont());
    s.setTextColor(p.meta, p.bg);
    if (gloss1 != nullptr) s.drawString(gloss1, MINI_MARGIN_X, glossY);
    if (gloss2 != nullptr) s.drawString(gloss2, MINI_MARGIN_X, glossY + MINI_BODY_LEAD);
}

QrArea drawQrPanel(Sprite& s, int x, int y, int moduleCount) {
    s.fillRoundRect(x, y, QR_SIZE, QR_SIZE, R2, MINI_PANEL);
    const int inner = QR_SIZE - 2 * QR_QUIET;
    const int modulePx = inner / moduleCount;
    const int code = modulePx * moduleCount;
    // Code centré : la quiet zone effective est ≥ QR_QUIET partout.
    const int cx = x + (QR_SIZE - code) / 2;
    const int cy = y + (QR_SIZE - code) / 2;
    return QrArea{cx, cy, modulePx};
}

}  // namespace daoa::ui
