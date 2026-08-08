#include "app.h"

#include <cstdio>
#include <cstring>

#include "theme.h"

namespace daoa {

namespace {

constexpr uint32_t kEnterFadeMs = 240;
constexpr uint32_t kRevealMs = 420;
constexpr uint32_t kAutoResultDelayMs = 800;
constexpr int kCenterX = theme::kWidth / 2;

struct Strings {
    const char* subtitle;
    const char* startHint;
    const char* castHintFmt;  // %d = numéro de la prochaine ligne
    const char* castDoneHint;
    const char* figureCaption;
    const char* complementCaption;
    const char* stableCaption;
    const char* keysHint;
};

constexpr Strings kStringsFr{
    "Un tirage du Yi Jing",
    "espace · commencer",
    "espace · tirer la ligne %d / 6",
    "espace · lire la figure",
    "figure obtenue",
    "figure complémentaire",
    "figure stable · sans mutation",
    "l · esc",
};

constexpr Strings kStringsEn{
    "A Yi Jing casting",
    "space · begin",
    "space · cast line %d / 6",
    "space · read the figure",
    "the figure received",
    "complementary figure",
    "stable figure · no change",
    "l · esc",
};

const Strings& str(Lang l) { return l == Lang::kFr ? kStringsFr : kStringsEn; }

// Découpe un texte UTF-8 sur les espaces et le dessine sur plusieurs lignes.
// Renvoie le nombre de lignes utilisées.
int drawWrapped(lgfx::LGFX_Sprite& s, const char* text, int x, int y,
                int maxWidth, int lineH, int maxLines) {
    char line[192] = {0};
    int lineLen = 0;
    int lines = 0;
    const char* p = text;
    while (*p != '\0' && lines < maxLines) {
        const char* wordEnd = p;
        while (*wordEnd != '\0' && *wordEnd != ' ') ++wordEnd;
        const int wordLen = static_cast<int>(wordEnd - p);

        char candidate[192];
        if (lineLen == 0) {
            snprintf(candidate, sizeof(candidate), "%.*s", wordLen, p);
        } else {
            snprintf(candidate, sizeof(candidate), "%s %.*s", line, wordLen, p);
        }
        if (s.textWidth(candidate) <= maxWidth || lineLen == 0) {
            snprintf(line, sizeof(line), "%s", candidate);
            lineLen = static_cast<int>(strlen(line));
        } else {
            s.drawString(line, x, y + lines * lineH);
            ++lines;
            snprintf(line, sizeof(line), "%.*s", wordLen, p);
            lineLen = static_cast<int>(strlen(line));
        }
        p = (*wordEnd == ' ') ? wordEnd + 1 : wordEnd;
    }
    if (lineLen > 0 && lines < maxLines) {
        s.drawString(line, x, y + lines * lineH);
        ++lines;
    }
    return lines;
}

}  // namespace

void App::switchTo(Screen s, uint32_t now) {
    screen_ = s;
    enter_.start(now, kEnterFadeMs);
    if (s == Screen::kCasting) {
        session_.reset();
        revealIndex_ = -1;
        completeAtMs_ = 0;
    }
    if (s == Screen::kResult) resultPage_ = 0;
}

void App::handleKey(Key k, uint32_t now) {
    if (k == Key::kLang) {
        lang_ = (lang_ == Lang::kFr) ? Lang::kEn : Lang::kFr;
        return;
    }
    if (k == Key::kReset) {
        switchTo(Screen::kWelcome, now);
        return;
    }

    switch (screen_) {
        case Screen::kWelcome:
            if (k == Key::kAction) switchTo(Screen::kCasting, now);
            break;

        case Screen::kCasting: {
            if (k != Key::kAction) break;
            // Préserver le rythme : pas de nouveau tirage pendant la révélation.
            if (revealIndex_ >= 0 && !reveal_.done(now)) break;
            if (session_.isComplete()) {
                switchTo(Screen::kResult, now);
                break;
            }
            session_.castNextLine();
            revealIndex_ = static_cast<int>(session_.lineCount()) - 1;
            reveal_.start(now, kRevealMs);
            if (session_.isComplete()) completeAtMs_ = now + kRevealMs;
            break;
        }

        case Screen::kResult:
            if (k == Key::kNext || k == Key::kPrev || k == Key::kAction) {
                resultPage_ = resultPage_ == 0 ? 1 : 0;
                enter_.start(now, 180);
            }
            break;
    }
}

bool App::ensureFrame() {
    if (frameReady_) return true;
    frame_.setColorDepth(16);
    frameReady_ = frame_.createSprite(theme::kWidth, theme::kHeight) != nullptr;
    return frameReady_;
}

uint16_t App::fade(uint16_t c, uint32_t now) const {
    return anim::lerp565(theme::kPaper, c, anim::easeOutCubic(enter_.progress(now)));
}

void App::renderFrame(uint32_t now) {
    if (!ensureFrame()) return;
    frame_.fillSprite(theme::kPaper);
    frame_.setTextWrap(false);
    switch (screen_) {
        case Screen::kWelcome: drawWelcome(now); break;
        case Screen::kCasting: drawCasting(now); break;
        case Screen::kResult: drawResult(now); break;
    }
}

void App::render(lgfx::LGFX_Device& dev, uint32_t now) {
    renderFrame(now);
    if (frameReady_) frame_.pushSprite(&dev, 0, 0);
}

// ---------------------------------------------------------------- écrans

void App::drawWelcome(uint32_t now) {
    auto& s = frame_;
    const uint16_t ink = fade(theme::kInk, now);
    const uint16_t stone = fade(theme::kStone, now);

    s.setTextDatum(textdatum_t::middle_center);

    s.setFont(&fonts::DejaVu40);
    s.setTextColor(ink, theme::kPaper);
    s.drawString("Daoa", kCenterX, 50);

    s.setFont(&fonts::efontJA_12);
    s.setTextColor(stone, theme::kPaper);
    s.drawString(str(lang_).subtitle, kCenterX, 82);

    // Respiration lente de l'invite.
    const float breath = 0.35f + 0.65f * anim::pulse(now, 2800);
    const uint16_t hint = anim::lerp565(theme::kPaper, theme::kStone, breath);
    s.setFont(&fonts::efontJA_10);
    s.setTextColor(fade(hint, now), theme::kPaper);
    s.drawString(str(lang_).startHint, kCenterX, 122);
}

void App::drawCastLine(int index, LineValue v, float t, uint32_t now) {
    auto& s = frame_;
    const int y = 10 + (5 - index) * theme::kLineSpacing;
    const int h = theme::kLineHeight;
    const int cy = y + h / 2;
    const float e = anim::easeOutQuint(t);
    const uint16_t col = fade(anim::lerp565(theme::kPaper, theme::kInk, 0.2f + 0.8f * e), now);

    if (isYang(v)) {
        const int w = static_cast<int>(theme::kLineWidth * e);
        if (w > 0) s.fillRect(kCenterX - w / 2, y, w, h, col);
    } else {
        const int segFull = (theme::kLineWidth - theme::kLineGap) / 2;
        const int seg = static_cast<int>(segFull * e);
        if (seg > 0) {
            // Les deux segments croissent du centre vers l'extérieur.
            s.fillRect(kCenterX - theme::kLineGap / 2 - seg, y, seg, h, col);
            s.fillRect(kCenterX + theme::kLineGap / 2, y, seg, h, col);
        }
    }

    // Marque de mutation, après la pose du trait.
    if (isMoving(v) && t > 0.7f) {
        const float mt = anim::clamp01((t - 0.7f) / 0.3f);
        const uint16_t mc = fade(anim::lerp565(theme::kPaper, theme::kAccent, mt), now);
        if (v == LineValue::kOldYang) {
            s.drawCircle(kCenterX, cy, 5, mc);
            s.drawCircle(kCenterX, cy, 6, mc);
        } else {
            s.drawLine(kCenterX - 4, cy - 4, kCenterX + 4, cy + 4, mc);
            s.drawLine(kCenterX - 4, cy + 4, kCenterX + 4, cy - 4, mc);
        }
    }
}

void App::drawCasting(uint32_t now) {
    auto& s = frame_;

    // Repères discrets des six emplacements.
    for (int i = 0; i < 6; ++i) {
        const int y = 10 + (5 - i) * theme::kLineSpacing + theme::kLineHeight / 2;
        const uint16_t tick = fade(anim::lerp565(theme::kPaper, theme::kStone, 0.35f), now);
        s.fillRect(kCenterX - 1, y - 1, 2, 2, tick);
    }

    const size_t count = session_.lineCount();
    for (size_t i = 0; i < count; ++i) {
        const float t = (static_cast<int>(i) == revealIndex_) ? reveal_.progress(now) : 1.f;
        drawCastLine(static_cast<int>(i), session_.line(i), t, now);
    }

    char hint[64];
    if (session_.isComplete()) {
        snprintf(hint, sizeof(hint), "%s", str(lang_).castDoneHint);
        // Passage automatique au résultat après un temps de lecture.
        if (completeAtMs_ != 0 && now > completeAtMs_ + kAutoResultDelayMs) {
            switchTo(Screen::kResult, now);
            return;
        }
    } else {
        snprintf(hint, sizeof(hint), str(lang_).castHintFmt,
                 static_cast<int>(count) + 1);
    }
    s.setTextDatum(textdatum_t::middle_center);
    s.setFont(&fonts::efontJA_10);
    s.setTextColor(fade(theme::kStone, now), theme::kPaper);
    s.drawString(hint, kCenterX, 124);
}

void App::drawMiniFigure(int x, int yTop, bool complementary, uint32_t now) {
    auto& s = frame_;
    const int w = 40, h = 4, gap = 10, spacing = 7;
    const uint16_t ink = fade(theme::kInk, now);
    const uint16_t accent = fade(theme::kAccent, now);

    for (int i = 0; i < 6; ++i) {
        LineValue v = session_.line(static_cast<size_t>(i));
        const bool moving = isMoving(v);
        if (complementary) v = mutated(v);
        const int y = yTop + (5 - i) * spacing;
        if (isYang(v)) {
            s.fillRect(x, y, w, h, ink);
        } else {
            const int seg = (w - gap) / 2;
            s.fillRect(x, y, seg, h, ink);
            s.fillRect(x + w - seg, y, seg, h, ink);
        }
        if (!complementary && moving) {
            s.fillCircle(x + w + 6, y + h / 2, 2, accent);
        }
    }
}

void App::drawResult(uint32_t now) {
    auto& s = frame_;
    const bool complementary = resultPage_ == 1;
    const bool stable = !session_.hasMovingLines();
    const uint8_t number =
        complementary ? session_.complementaryNumber() : session_.originalNumber();
    const HexagramInfo& h = hexagram(number);
    const Strings& t = str(lang_);

    const uint16_t ink = fade(theme::kInk, now);
    const uint16_t stone = fade(theme::kStone, now);

    s.setTextDatum(textdatum_t::top_left);

    // En-tête : numéro et pinyin.
    char header[48];
    snprintf(header, sizeof(header), "%d · %s", h.number, h.pinyin);
    s.setFont(&fonts::efontJA_12);
    s.setTextColor(stone, theme::kPaper);
    s.drawString(header, 12, 10);

    // Caractère chinois. efontCN couvre 61 figures ; 遯 (33), 夬 (43) et
    // 姤 (44) n'existent qu'en efontJA. Fonte dédiée aux 64 glyphes prévue
    // en phase 1.5 (docs/05_ROADMAP.md).
    const bool cnMissing = number == 33 || number == 43 || number == 44;
    if (cnMissing) {
        s.setFont(&fonts::efontJA_24_b);
    } else {
        s.setFont(&fonts::efontCN_24_b);
    }
    s.setTextColor(ink, theme::kPaper);
    s.drawString(h.hanzi, 12, 26);

    // Titre, à gauche de la mini-figure.
    s.setFont(&fonts::efontJA_16_b);
    s.setTextColor(ink, theme::kPaper);
    const int titleLines = drawWrapped(s, title(h, lang_), 12, 56, 160, 18, 2);

    // Description.
    const int descY = 56 + titleLines * 18 + 6;
    const int maxLines = (120 - descY) / 14;
    s.setFont(&fonts::efontJA_12);
    s.setTextColor(ink, theme::kPaper);
    drawWrapped(s, description(h, lang_), 12, descY, 216, 14, maxLines > 0 ? maxLines : 1);

    drawMiniFigure(184, 12, complementary, now);

    const char* caption = complementary
                              ? (stable ? t.stableCaption : t.complementCaption)
                              : t.figureCaption;
    drawBottomBar(caption, true, now);
}

void App::drawBottomBar(const char* caption, bool showDots, uint32_t now) {
    auto& s = frame_;
    const uint16_t stone = fade(theme::kStone, now);

    s.setFont(&fonts::efontJA_10);
    s.setTextColor(stone, theme::kPaper);
    s.setTextDatum(textdatum_t::bottom_left);
    s.drawString(caption, 12, 134);
    s.setTextDatum(textdatum_t::bottom_right);
    s.drawString(str(lang_).keysHint, 228, 134);

    if (showDots) {
        const uint16_t active = fade(theme::kInk, now);
        const uint16_t inactive = fade(anim::lerp565(theme::kPaper, theme::kStone, 0.45f), now);
        s.fillCircle(kCenterX - 5, 129, 2, resultPage_ == 0 ? active : inactive);
        s.fillCircle(kCenterX + 5, 129, 2, resultPage_ == 1 ? active : inactive);
    }
}

}  // namespace daoa
