#include "app.h"

#include <cstdio>
#include <cstring>

#include <vector>

#include "../content/trigrams.h"
#include "assets/wordmark_data.h"
#include "qrcode.h"
#include "text_rules.h"

namespace daoa {

namespace {

using namespace daoa::tokens;

constexpr uint32_t kEnterFadeMs = 240;
constexpr uint32_t kRevealMs = 420;
constexpr uint32_t kAutoResultDelayMs = 800;

// URL encodée dans le QR promo ; le site route lui-même FR/EN.
constexpr const char* kPromoUrl = "https://daoa.app";
constexpr uint8_t kQrVersion = 2;  // 25 modules

// Lettermark : gabarit accueil, largeur 104, à (68, 26) pour une boîte de
// 44 px de haut — le masque recadré fait 36 px, d'où l'ordonnée optique.
constexpr int kWordmarkX = 68;
constexpr int kWordmarkY = 26 + (44 - kWordmarkHeight) / 2;
inline constexpr MaskAsset kWordmarkMask{kWordmarkWidth, kWordmarkHeight, kWordmarkAlpha};

// Gabarits (Screens.jsx du design system).
constexpr int kSubtitleY = 78;
constexpr int kOrnamentX = 50, kOrnamentY = 96;
constexpr int kWelcomePromptY = 112;
constexpr int kCastingX = 60, kCastingY = 22, kCastingPromptY = 108;
constexpr int kFigureTitleTop = 24;
constexpr int kMiniFigureX = 174, kMiniFigureY = 24;
constexpr int kSealX = 6, kSealY = 12;
constexpr int kPromoNameY = 64, kPromoTagY = 86, kPromoEscY = 118;
constexpr int kQrPanelX = 118, kQrPanelY = 10;

struct HelpSection {
    const char* title;
    const char* body;
};

struct Strings {
    const char* subtitle;
    const char* helpHint;
    const char* langHint;
    const char* castLineFmt;    // invite tirage, %d = prochaine ligne
    const char* castDoneText;   // invite fin de tirage
    const char* promoTag1;
    const char* promoTag2;
    const char* posUpper;       // écran structure
    const char* posLower;
    const char* detailPortrait;
    const char* detailJudgment;
    const char* detailImage;
    const char* const* detailLineTitles;  // 6 titres, bas → haut
    const char* helpTitle;
    const HelpSection* helpSections;      // 6 sections, la dernière = touches
    const char* const* helpKeys;          // 5 lignes
};

// Aide embarquée : adaptation condensée de l'écran « Comprendre » de
// Daoa 2 (Localizable.xcstrings, clés understand.*), réécrite pour cette
// plateforme — ici les lignes mouvantes sont vivantes (tirage ligne à
// ligne), et une section décrit les touches.
constexpr HelpSection kHelpFr[] = {
    {"le yi jing",
     "Le Livre des Transformations est l'un des plus anciens textes de sagesse "
     "de la Chine. Soixante-quatre figures y décrivent les mouvements du "
     "changement. Daoa s'en inspire pour une réflexion fondée sur le "
     "discernement, non sur la prédiction."},
    {"le tirage",
     "Six lignes sont tirées une à une, de bas en haut, selon la méthode "
     "traditionnelle des trois pièces. Le hasard est réel : il vient d'un "
     "générateur matériel, et rien ne l'influence."},
    {"les lignes mouvantes",
     "Un vieux yang (cercle) ou un vieux yin (croix) est une ligne en "
     "transformation. Ces lignes dessinent une seconde figure, celle vers "
     "laquelle la situation tend — la figure complémentaire."},
    {"interpréter",
     "Le Yi Jing ne prédit pas l'avenir : il offre une vision symbolique du "
     "présent, un miroir pour observer les forces en mouvement et avancer "
     "avec plus de justesse."},
    {"les textes",
     "Les textes de chaque figure sont des formulations propres à Daoa, "
     "fidèles aux traductions du domaine public de James Legge (1882) et "
     "Charles de Harlez (1889)."},
    {"les touches", nullptr},
};

constexpr HelpSection kHelpEn[] = {
    {"the yi jing",
     "The Book of Changes is one of the oldest wisdom texts of ancient "
     "China. Sixty-four figures describe the movements of change. Daoa "
     "draws on it for reflection grounded in discernment, not prediction."},
    {"the casting",
     "Six lines are cast one by one, from bottom to top, by the traditional "
     "three-coins method. The chance is real: it comes from a hardware "
     "generator, and nothing influences it."},
    {"the moving lines",
     "An old yang (circle) or old yin (cross) is a line in transformation. "
     "These lines trace a second figure, the one the situation is tending "
     "toward — the complementary figure."},
    {"interpreting",
     "The Yi Jing does not predict the future: it offers a symbolic view of "
     "the present, a mirror for observing the forces in motion and moving "
     "forward with better judgment."},
    {"the texts",
     "Each figure's texts are Daoa's own wording, faithful to the "
     "public-domain translations of James Legge (1882) and Charles de "
     "Harlez (1889)."},
    {"the keys", nullptr},
};

// Colonnes alignées sur l'avance de 6 px (fonte monochasse).
constexpr const char* kHelpKeysFr[] = {
    "espace   tirer une ligne, avancer",
    "← →      les deux figures",
    "↓        texte complet · ↑ ↓ pages",
    "L        langue · h  cette aide",
    "esc      revenir en arrière",
};
constexpr const char* kHelpKeysEn[] = {
    "space    cast a line, continue",
    "← →      the two figures",
    "↓        full text · ↑ ↓ pages",
    "L        language · h  this help",
    "esc      go back",
};

constexpr const char* kLineTitlesFr[6] = {
    "trait 1 · en bas", "trait 2", "trait 3", "trait 4", "trait 5", "trait 6 · en haut",
};
constexpr const char* kLineTitlesEn[6] = {
    "line 1 · bottom", "line 2", "line 3", "line 4", "line 5", "line 6 · top",
};

constexpr Strings kStringsFr{
    "Un tirage du Yi Jing",
    "h · aide",
    "L · english",
    "tirer la ligne %d / 6",
    "lire la figure",
    "L'app complète",
    "du Yi Jing",
    "haut",
    "bas",
    "portrait",
    "jugement",
    "image",
    kLineTitlesFr,
    "aide",
    kHelpFr,
    kHelpKeysFr,
};

constexpr Strings kStringsEn{
    "A Yi Jing casting",
    "h · help",
    "L · français",
    "cast line %d / 6",
    "read the figure",
    "The complete",
    "Yi Jing app",
    "upper",
    "lower",
    "portrait",
    "judgment",
    "image",
    kLineTitlesEn,
    "help",
    kHelpEn,
    kHelpKeysEn,
};

const Strings& str(Lang l) { return l == Lang::kFr ? kStringsFr : kStringsEn; }

// Fonte du sinogramme : efontCN, repli efontJA pour 遯 (33), 夬 (43) et
// 姤 (44), absents d'efontCN. Fonte dédiée prévue (docs/05_ROADMAP.md).
const lgfx::IFont* glyphFont(uint8_t number) {
    const bool cnMissing = number == 33 || number == 43 || number == 44;
    return cnMissing ? static_cast<const lgfx::IFont*>(&fonts::efontJA_24_b)
                     : static_cast<const lgfx::IFont*>(&fonts::efontCN_24_b);
}

// Convertit les lignes de la session en traits du composant Hexagram.
void sessionLines(const CastSession& session, bool complementary, ui::HexLine* out) {
    for (size_t i = 0; i < CastSession::kLineCount; ++i) {
        LineValue v = i < session.lineCount() ? session.line(i) : LineValue::kYoungYang;
        const bool moving = isMoving(v);
        if (complementary) v = mutated(v);
        out[i] = ui::HexLine{isYang(v), moving && !complementary};
    }
}

// Enveloppe un texte en lignes de colonne pleine (mesure fonte réelle).
std::vector<std::string> wrapLines(lgfx::LGFX_Sprite& s, const char* text, int maxW) {
    s.setFont(&fonts::efontJA_12);
    std::vector<std::string> lines;
    std::string line;
    const char* p = text;
    char candidate[256];
    const char* lineStart = text;
    int lineLen = 0;
    while (*p != '\0') {
        const char* wordEnd = p;
        while (*wordEnd != '\0' && *wordEnd != ' ') ++wordEnd;
        snprintf(candidate, sizeof(candidate), "%.*s",
                 static_cast<int>(wordEnd - lineStart), lineStart);
        if (lineLen == 0 || s.textWidth(candidate) <= maxW) {
            lineLen = static_cast<int>(wordEnd - lineStart);
        } else {
            lines.emplace_back(lineStart, static_cast<size_t>(lineLen));
            lineStart = p;
            lineLen = static_cast<int>(wordEnd - p);
        }
        p = (*wordEnd == ' ') ? wordEnd + 1 : wordEnd;
    }
    if (lineLen > 0) lines.emplace_back(lineStart, static_cast<size_t>(lineLen));
    return lines;
}

}  // namespace

void App::switchTo(Screen s, uint32_t now) {
    const Screen previous = screen_;
    screen_ = s;
    enter_.start(now, kEnterFadeMs);
    if (s == Screen::kCasting) {
        session_.reset();
        revealIndex_ = -1;
        completeAtMs_ = 0;
    }
    // Ne pas revenir en page 0 quand on rentre de la lecture détaillée
    // ni de l'aide.
    if (s == Screen::kResult && previous != Screen::kDetail && previous != Screen::kHelp) {
        resultPage_ = 0;
    }
    if (s == Screen::kDetail) buildDetailDoc();
    if (s == Screen::kHelp) buildHelpDoc();
}

void App::handleKey(Key k, uint32_t now) {
    if (k == Key::kLang) {
        lang_ = (lang_ == Lang::kFr) ? Lang::kEn : Lang::kFr;
        if (screen_ == Screen::kDetail) buildDetailDoc();
        if (screen_ == Screen::kHelp) buildHelpDoc();
        return;
    }
    if (k == Key::kHelp) {
        if (screen_ == Screen::kHelp) {
            switchTo(helpReturn_, now);
        } else {
            helpReturn_ = screen_;
            switchTo(Screen::kHelp, now);
        }
        return;
    }
    if (k == Key::kReset) {
        // esc contextuel : l'aide revient d'où l'on vient, la lecture
        // détaillée au résultat, partout ailleurs à l'accueil.
        if (screen_ == Screen::kHelp) {
            switchTo(helpReturn_, now);
        } else if (screen_ == Screen::kDetail || screen_ == Screen::kStructure) {
            switchTo(Screen::kResult, now);
        } else {
            switchTo(Screen::kWelcome, now);
        }
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
            if (k == Key::kNext || k == Key::kPrev) {
                resultPage_ = resultPage_ == 0 ? 1 : 0;
                enter_.start(now, 180);
            } else if (k == Key::kDown) {
                detailNumber_ = resultPage_ == 1 ? session_.complementaryNumber()
                                                 : session_.originalNumber();
                switchTo(Screen::kDetail, now);
            } else if (k == Key::kAction) {
                switchTo(Screen::kPromo, now);
            }
            break;

        case Screen::kDetail:
        case Screen::kHelp: {
            const int pageCount = docPageCount();
            if (k == Key::kDown || k == Key::kAction || k == Key::kNext) {
                if (docPage_ < pageCount - 1) {
                    ++docPage_;
                    enter_.start(now, 150);
                } else if (k == Key::kDown && screen_ == Screen::kDetail) {
                    // ↓ après la dernière page de texte → structure.
                    switchTo(Screen::kStructure, now);
                }
            } else if (k == Key::kUp || k == Key::kPrev) {
                if (docPage_ > 0) {
                    --docPage_;
                    enter_.start(now, 150);
                }
            }
            break;
        }

        case Screen::kStructure:
            if (k == Key::kUp) {
                // Remonte dans le texte, sur sa dernière page.
                switchTo(Screen::kDetail, now);
                docPage_ = docPageCount() - 1;
            }
            break;

        case Screen::kPromo:
            if (k == Key::kAction) switchTo(Screen::kWelcome, now);
            break;
    }
}

bool App::ensureFrame() {
    if (frameReady_) return true;
    frame_.setColorDepth(16);
    frameReady_ = frame_.createSprite(MINI_W, MINI_H) != nullptr;
    return frameReady_;
}

uint16_t App::fade(uint16_t c, uint32_t now) const {
    return anim::lerp565(MINI_BG, c, anim::easeOutCubic(enter_.progress(now)));
}

ui::Palette App::palette(uint32_t now) const {
    return ui::Palette{MINI_BG, fade(MINI_TEXT, now), fade(MINI_TEXT_META, now),
                       fade(MINI_MUTATION, now), fade(MINI_BRAND, now)};
}

void App::renderFrame(uint32_t now) {
    if (!ensureFrame()) return;
    frame_.fillSprite(MINI_BG);
    frame_.setTextWrap(false);
    switch (screen_) {
        case Screen::kWelcome: drawWelcome(now); break;
        case Screen::kCasting: drawCasting(now); break;
        case Screen::kResult: drawResult(now); break;
        case Screen::kDetail: drawDetail(now); break;
        case Screen::kStructure: drawStructure(now); break;
        case Screen::kPromo: drawPromo(now); break;
        case Screen::kHelp: drawHelp(now); break;
    }
}

void App::render(lgfx::LGFX_Device& dev, uint32_t now) {
    renderFrame(now);
    if (frameReady_) frame_.pushSprite(&dev, 0, 0);
}

// ---------------------------------------------------------------- écrans

void App::drawWelcome(uint32_t now) {
    auto& s = frame_;
    const ui::Palette p = palette(now);
    const Strings& t = str(lang_);
    const float enter = anim::easeOutCubic(enter_.progress(now));

    ui::drawCornerHints(s, p, t.helpHint, t.langHint);

    // Un seul accent : le lettermark.
    ui::drawMask(s, kWordmarkMask, kWordmarkX, kWordmarkY, MINI_BRAND, MINI_BG, enter);

    s.setFont(&fonts::efontJA_12);
    s.setTextColor(p.meta, p.bg);
    s.setTextDatum(textdatum_t::top_center);
    s.drawString(t.subtitle, MINI_W / 2, kSubtitleY);
    s.setTextDatum(textdatum_t::top_left);

    ui::drawMask(s, kMaskOrnementAccueil, kOrnamentX, kOrnamentY, MINI_TEXT_META, MINI_BG,
                 enter);

    // Respiration lente de l'invite (la touche reste pleine).
    const float breath = 0.45f + 0.55f * anim::pulse(now, 2800);
    ui::Palette pp = p;
    pp.meta = fade(anim::lerp565(MINI_BG, MINI_TEXT_META, breath), now);
    const ui::PromptSeg segs[] = {{"space", nullptr}, {nullptr, "begin"}};
    ui::drawPrompt(s, pp, lang_, kWelcomePromptY, segs, 2);
}

void App::drawCasting(uint32_t now) {
    auto& s = frame_;
    const ui::Palette p = palette(now);
    const Strings& t = str(lang_);

    ui::HexLine lines[6];
    sessionLines(session_, false, lines);
    const int revealed = static_cast<int>(session_.lineCount());
    const float lastReveal = (revealIndex_ >= 0) ? reveal_.progress(now) : 1.f;
    ui::drawHexagram(s, p, kCastingX, kCastingY, lines, revealed, ui::YAO_L, true,
                     lastReveal);

    char text[48];
    if (session_.isComplete()) {
        snprintf(text, sizeof(text), "%s", t.castDoneText);
        if (completeAtMs_ != 0 && now > completeAtMs_ + kAutoResultDelayMs) {
            switchTo(Screen::kResult, now);
            return;
        }
    } else {
        snprintf(text, sizeof(text), t.castLineFmt, revealed + 1);
    }
    const ui::PromptSeg segs[] = {{"space", nullptr}, {nullptr, text}};
    ui::drawPrompt(s, p, lang_, kCastingPromptY, segs, 2);
}

void App::drawResult(uint32_t now) {
    auto& s = frame_;
    const ui::Palette p = palette(now);
    const bool complementary = resultPage_ == 1;
    const bool stable = !session_.hasMovingLines();
    const uint8_t number =
        complementary ? session_.complementaryNumber() : session_.originalNumber();
    const HexagramInfo& h = hexagram(number);
    const Strings& t = str(lang_);

    char context[48];
    snprintf(context, sizeof(context), "%d · %s", h.number, h.pinyin);
    ui::drawDocHeader(s, p, context, resultPage_ + 1, 2);

    const std::vector<std::string> gloss =
        clampGloss(wrapLines(s, description(h, lang_), MINI_COL_W));
    ui::drawFigureTitle(s, p, h.hanzi, glyphFont(number), title(h, lang_),
                        gloss.empty() ? nullptr : gloss[0].c_str(),
                        gloss.size() > 1 ? gloss[1].c_str() : nullptr, kFigureTitleTop);

    ui::HexLine lines[6];
    sessionLines(session_, complementary, lines);
    ui::drawHexagram(s, p, kMiniFigureX, kMiniFigureY, lines, 6, ui::YAO_M,
                     !complementary);

    (void)t;
    const ui::FooterId footer = complementary
                                    ? (stable ? ui::kFooterResultStable : ui::kFooterResultOther)
                                    : ui::kFooterResultDrawn;
    ui::drawFooterBar(s, p, lang_, ui::kFooterSpecs[footer]);
}

int App::docPageCount() const {
    return static_cast<int>((doc_.size() + MINI_READER_LINES - 1) / MINI_READER_LINES);
}

void App::drawDocScreen(const char* context, ui::FooterId footer, uint32_t now) {
    auto& s = frame_;
    const ui::Palette p = palette(now);

    ui::drawDocHeader(s, p, context, docPage_ + 1, docPageCount());

    const size_t first = static_cast<size_t>(docPage_) * MINI_READER_LINES;
    const int count = static_cast<int>(
        doc_.size() - first < MINI_READER_LINES ? doc_.size() - first : MINI_READER_LINES);
    if (first < doc_.size()) ui::drawReader(s, p, doc_.data() + first, count);

    ui::drawFooterBar(s, p, lang_, ui::kFooterSpecs[footer]);
}

void App::drawDetail(uint32_t now) {
    const HexagramInfo& h = hexagram(detailNumber_);
    char context[96];
    snprintf(context, sizeof(context), "%d · %s · %s", h.number, h.pinyin, title(h, lang_));
    drawDocScreen(context, ui::kFooterDoc, now);
}

void App::drawHelp(uint32_t now) {
    drawDocScreen(str(lang_).helpTitle, ui::kFooterHelp, now);
}

// Écran Structure (HANDOFF-v2, tâche 3) : les deux trigrammes de la figure,
// dérivés de ses traits. Rangées aux ordonnées 38 (haut) et 74 (bas).
void App::drawStructure(uint32_t now) {
    auto& s = frame_;
    const ui::Palette p = palette(now);
    const Strings& t = str(lang_);
    const HexagramInfo& h = hexagram(detailNumber_);

    char context[96];
    snprintf(context, sizeof(context), "%d · %s · %s", h.number, h.pinyin,
             keyLabel("structure", lang_));
    ui::drawDocHeader(s, p, context, 1, 1);

    static const MaskAsset* kTrigramMasks[8] = {
        &kMaskTrigramme1, &kMaskTrigramme2, &kMaskTrigramme3, &kMaskTrigramme4,
        &kMaskTrigramme5, &kMaskTrigramme6, &kMaskTrigramme7, &kMaskTrigramme8,
    };
    struct Row {
        uint8_t bits;
        const char* position;
        int y;
    };
    // Haut = lignes 4-5-6, bas = lignes 1-2-3 (bits bas → haut).
    const Row rows[2] = {
        {static_cast<uint8_t>((h.linesBits >> 3) & 7), t.posUpper, 38},
        {static_cast<uint8_t>(h.linesBits & 7), t.posLower, 74},
    };
    s.setFont(&fonts::efontJA_12);
    for (const Row& row : rows) {
        const TrigramInfo& tri = trigramFromBits(row.bits);
        const MaskAsset& m = *kTrigramMasks[tri.maskIndex - 1];
        ui::drawMask(s, m, MINI_MARGIN_X, row.y, p.text, p.bg);
        char name[64];
        snprintf(name, sizeof(name), "%s · %s", tri.pinyin, trigramName(tri, lang_));
        const int textY = row.y + (m.height - MINI_BODY_H) / 2;
        s.setTextDatum(textdatum_t::top_left);
        s.setTextColor(p.text, p.bg);
        s.drawString(name, MINI_MARGIN_X + m.width + SP_8, textY);
        s.setTextDatum(textdatum_t::top_right);
        s.setTextColor(p.meta, p.bg);
        s.drawString(row.position, MINI_W - MINI_MARGIN_X, textY);
    }

    ui::drawFooterBar(s, p, lang_, ui::kFooterSpecs[ui::kFooterStructure]);
}

// Découpe un texte en lignes (pointeurs dans le corpus, aucune copie) à la
// largeur de colonne, avec la fonte du corps de texte.
void App::appendDocText(const char* text, bool meta) {
    auto& s = frame_;
    s.setFont(&fonts::efontJA_12);
    char candidate[256];
    const char* lineStart = text;
    int lineLen = 0;
    const char* p = text;
    while (*p != '\0') {
        const char* wordEnd = p;
        while (*wordEnd != '\0' && *wordEnd != ' ') ++wordEnd;
        snprintf(candidate, sizeof(candidate), "%.*s",
                 static_cast<int>(wordEnd - lineStart), lineStart);
        if (lineLen == 0 || s.textWidth(candidate) <= MINI_COL_W) {
            lineLen = static_cast<int>(wordEnd - lineStart);
        } else {
            doc_.push_back({lineStart, static_cast<uint16_t>(lineLen), meta});
            lineStart = p;
            lineLen = static_cast<int>(wordEnd - p);
        }
        p = (*wordEnd == ' ') ? wordEnd + 1 : wordEnd;
    }
    if (lineLen > 0) doc_.push_back({lineStart, static_cast<uint16_t>(lineLen), meta});
}

void App::buildDetailDoc() {
    if (!ensureFrame()) return;
    doc_.clear();
    docPage_ = 0;
    const HexagramDetail& d = detail(detailNumber_);
    const Strings& t = str(lang_);
    const bool fr = lang_ == Lang::kFr;

    appendDocText(t.detailPortrait, true);
    appendDocText(fr ? d.generalFr : d.generalEn, false);
    appendDocText(t.detailJudgment, true);
    appendDocText(fr ? d.judgmentFr : d.judgmentEn, false);
    appendDocText(t.detailImage, true);
    appendDocText(fr ? d.imageFr : d.imageEn, false);
    for (int i = 0; i < 6; ++i) {
        appendDocText(t.detailLineTitles[i], true);
        appendDocText(fr ? d.linesFr[i] : d.linesEn[i], false);
    }
}

void App::buildHelpDoc() {
    if (!ensureFrame()) return;
    doc_.clear();
    docPage_ = 0;
    const Strings& t = str(lang_);
    for (int i = 0; i < 6; ++i) {
        const HelpSection& sec = t.helpSections[i];
        appendDocText(sec.title, true);
        if (sec.body != nullptr) {
            appendDocText(sec.body, false);
        } else {
            // Dernière section : la liste des touches, une par ligne.
            for (int j = 0; j < 5; ++j) appendDocText(t.helpKeys[j], false);
            break;
        }
    }
}

void App::drawPromo(uint32_t now) {
    auto& s = frame_;
    const ui::Palette p = palette(now);
    const Strings& t = str(lang_);

    // Fond bitmap précomposé sur noir (grain, ≤ 8 % de luminance).
    s.setSwapBytes(true);
    s.pushImage(0, 0, kPromoBgWidth, kPromoBgHeight, kPromoBg565);
    s.setSwapBytes(false);

    ui::drawMask(s, kMaskSceau, kSealX, kSealY, p.brand, MINI_BG);

    s.setTextDatum(textdatum_t::top_left);
    s.setFont(&fonts::efontJA_16_b);
    s.setTextColor(p.text, p.bg);
    s.drawString("Daoa", MINI_MARGIN_X, kPromoNameY);

    s.setFont(&fonts::efontJA_12);
    s.setTextColor(p.meta, p.bg);
    s.drawString(t.promoTag1, MINI_MARGIN_X, kPromoTagY);
    s.drawString(t.promoTag2, MINI_MARGIN_X, kPromoTagY + MINI_BODY_LEAD);

    s.setTextColor(p.text, p.bg);
    s.drawString("esc", MINI_MARGIN_X, kPromoEscY);
    s.setTextColor(p.meta, p.bg);
    char home[24];
    snprintf(home, sizeof(home), " · %s", keyLabel("home", lang_));
    s.drawString(home, MINI_MARGIN_X + s.textWidth("esc"), kPromoEscY);

    // QR généré une seule fois (URL constante).
    static QRCode qr;
    static uint8_t qrData[128];  // ≥ qrcode_getBufferSize(2) = 79
    static bool qrReady = false;
    if (!qrReady) {
        qrcode_initText(&qr, qrData, kQrVersion, ECC_MEDIUM, kPromoUrl);
        qrReady = true;
    }
    const ui::QrArea area = ui::drawQrPanel(s, kQrPanelX, kQrPanelY, qr.size);
    for (uint8_t y = 0; y < qr.size; ++y) {
        for (uint8_t x = 0; x < qr.size; ++x) {
            if (qrcode_getModule(&qr, x, y)) {
                s.fillRect(area.x + x * area.modulePx, area.y + y * area.modulePx,
                           area.modulePx, area.modulePx, MINI_PANEL_INK);
            }
        }
    }
}

}  // namespace daoa
