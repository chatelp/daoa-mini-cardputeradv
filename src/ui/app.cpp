#include "app.h"

#include <cstdio>
#include <cstring>

#include "assets/wordmark_data.h"
#include "qrcode.h"

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
    const char* spaceKey;
    const char* helpHint;
    const char* langHint;
    const char* startText;      // invite accueil, après « espace »
    const char* castLineFmt;    // invite tirage, %d = prochaine ligne
    const char* castDoneText;   // invite fin de tirage
    const char* figureCaption;
    const char* complementCaption;
    const char* stableCaption;
    const char* textLabel;      // libellé du raccourci ↓
    const char* homeLabel;      // libellé esc sur l'aide
    const char* promoTag1;
    const char* promoTag2;
    const char* promoHome;      // « accueil » après esc sur la promo
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
    "espace",
    "h · aide",
    "L · english",
    "commencer",
    "tirer la ligne %d / 6",
    "lire la figure",
    "figure obtenue",
    "complémentaire",
    "figure stable",
    "texte",
    "accueil",
    "L'app complète",
    "du Yi Jing",
    "accueil",
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
    "space",
    "h · help",
    "L · français",
    "begin",
    "cast line %d / 6",
    "read the figure",
    "first figure",
    "complementary",
    "stable figure",
    "text",
    "home",
    "The full app",
    "of the Yi Jing",
    "home",
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

// Coupe gloss en deux lignes au plus (largeur pleine colonne), « … » si déborde.
void wrapGloss(lgfx::LGFX_Sprite& s, const char* text, char* line1, char* line2, size_t cap) {
    s.setFont(&fonts::efontJA_12);
    line1[0] = line2[0] = '\0';
    char* out[2] = {line1, line2};
    int lineIdx = 0;
    const char* lineStart = text;
    int lineLen = 0;
    const char* p = text;
    char candidate[256];
    while (*p != '\0' && lineIdx < 2) {
        const char* wordEnd = p;
        while (*wordEnd != '\0' && *wordEnd != ' ') ++wordEnd;
        snprintf(candidate, sizeof(candidate), "%.*s",
                 static_cast<int>(wordEnd - lineStart), lineStart);
        if (lineLen == 0 || s.textWidth(candidate) <= MINI_COL_W) {
            lineLen = static_cast<int>(wordEnd - lineStart);
        } else {
            snprintf(out[lineIdx], cap, "%.*s", lineLen, lineStart);
            ++lineIdx;
            lineStart = p;
            lineLen = static_cast<int>(wordEnd - p);
        }
        p = (*wordEnd == ' ') ? wordEnd + 1 : wordEnd;
    }
    if (lineIdx < 2 && lineLen > 0) {
        snprintf(out[lineIdx], cap, "%.*s", lineLen, lineStart);
    } else if (lineIdx >= 2 && *p != '\0') {
        // Déborde : la lecture complète est à une touche (↓).
        const size_t n = strlen(line2);
        if (n + 4 < cap) memcpy(line2 + n, "…", 4);
    }
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
        } else {
            switchTo(screen_ == Screen::kDetail ? Screen::kResult : Screen::kWelcome, now);
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
            const int pageCount =
                static_cast<int>((doc_.size() + MINI_READER_LINES - 1) / MINI_READER_LINES);
            if (k == Key::kDown || k == Key::kAction || k == Key::kNext) {
                if (docPage_ < pageCount - 1) {
                    ++docPage_;
                    enter_.start(now, 150);
                }
            } else if (k == Key::kUp || k == Key::kPrev) {
                if (docPage_ > 0) {
                    --docPage_;
                    enter_.start(now, 150);
                }
            }
            break;
        }

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
    const ui::PromptSeg segs[] = {{t.spaceKey, nullptr}, {nullptr, t.startText}};
    ui::drawPrompt(s, pp, kWelcomePromptY, segs, 2);
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
    const ui::PromptSeg segs[] = {{t.spaceKey, nullptr}, {nullptr, text}};
    ui::drawPrompt(s, p, kCastingPromptY, segs, 2);
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

    char gloss1[160], gloss2[160];
    wrapGloss(s, description(h, lang_), gloss1, gloss2, sizeof(gloss1));
    ui::drawFigureTitle(s, p, h.hanzi, glyphFont(number), title(h, lang_), gloss1,
                        gloss2[0] ? gloss2 : nullptr, kFigureTitleTop);

    ui::HexLine lines[6];
    sessionLines(session_, complementary, lines);
    ui::drawHexagram(s, p, kMiniFigureX, kMiniFigureY, lines, 6, ui::YAO_M,
                     !complementary);

    const char* legend = complementary
                             ? (stable ? t.stableCaption : t.complementCaption)
                             : t.figureCaption;
    const ui::FooterAction actions[] = {
        {"↓", t.textLabel}, {t.spaceKey, nullptr}, {"esc", nullptr}};
    ui::drawFooterBar(s, p, legend, actions, 3);
}

void App::drawDocScreen(const char* context, const char* footerLegend, uint32_t now) {
    auto& s = frame_;
    const ui::Palette p = palette(now);
    const int pageCount =
        static_cast<int>((doc_.size() + MINI_READER_LINES - 1) / MINI_READER_LINES);

    ui::drawDocHeader(s, p, context, docPage_ + 1, pageCount);

    const size_t first = static_cast<size_t>(docPage_) * MINI_READER_LINES;
    const int count = static_cast<int>(
        doc_.size() - first < MINI_READER_LINES ? doc_.size() - first : MINI_READER_LINES);
    if (first < doc_.size()) ui::drawReader(s, p, doc_.data() + first, count);

    if (footerLegend != nullptr) {
        const ui::FooterAction actions[] = {{"esc", str(lang_).homeLabel}};
        ui::drawFooterBar(s, p, footerLegend, actions, 1);
    } else {
        const ui::FooterAction actions[] = {{"↑↓", nullptr}, {"esc", nullptr}};
        ui::drawFooterBar(s, p, nullptr, actions, 2);
    }
}

void App::drawDetail(uint32_t now) {
    const HexagramInfo& h = hexagram(detailNumber_);
    char context[96];
    snprintf(context, sizeof(context), "%d · %s · %s", h.number, h.pinyin, title(h, lang_));
    drawDocScreen(context, nullptr, now);
}

void App::drawHelp(uint32_t now) { drawDocScreen(str(lang_).helpTitle, "", now); }

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
    snprintf(home, sizeof(home), " · %s", t.promoHome);
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
