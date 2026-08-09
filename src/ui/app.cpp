#include "app.h"

#include <cmath>
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
    const char* settingsHint;   // coin bas-gauche de l'accueil
    const char* fieldHint;      // coin bas-droite du tirage (touche w)
    const char* castLineFmt;    // invite tirage, %d = prochaine ligne
    const char* castDoneText;   // invite fin de tirage
    const char* promoTag1;
    const char* promoTag2;
    const char* posUpper;       // écran structure
    const char* posLower;
    const char* scanText;       // invite pendant la lecture du lieu
    const char* fieldTitle;     // en-tête de la vue technique
    const char* fieldOffText;   // vue technique quand la lecture est coupée
    const char* rowAmbient;     // réglages : nom de la ligne
    const char* rowSound;
    const char* stateOnAmbient;
    const char* stateOffAmbient;
    const char* stateOnSound;
    const char* stateOffSound;
    const char* const* ambientBody;  // 3 lignes d'explication honnête
    const char* detailGloss;    // section « glose » quand elle est tronquée
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
    {"la lecture du lieu",
     "Avant chaque tirage, l'appareil écoute le lieu : un scan WiFi passif, "
     "jamais connecté, dont rien n'est gardé. La densité du champ penche "
     "yin ou yang (± 5 % au plus), son agitation rend les mutations un peu "
     "plus probables (+ 4 % au plus). Le hasard matériel reste maître. "
     "w affiche le détail technique ; s permet de couper la lecture."},
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
    {"reading the place",
     "Before each casting the device listens to the place: a passive WiFi "
     "scan, never connected, nothing kept. The field's density leans yin "
     "or yang (± 5% at most), its agitation makes changes slightly more "
     "likely (+ 4% at most). Hardware chance remains the master. "
     "w shows the technical detail; s can turn the reading off."},
    {"the keys", nullptr},
};

// Colonnes alignées sur l'avance de 6 px (fonte monochasse).
constexpr const char* kHelpKeysFr[] = {
    "espace   tirer une ligne, avancer",
    "← →      les deux figures",
    "↓        texte complet · ↑ ↓ pages",
    "L        langue · h  cette aide",
    "s        réglages · w  champ technique",
    "esc      revenir en arrière",
};
constexpr const char* kHelpKeysEn[] = {
    "space    cast a line, continue",
    "← →      the two figures",
    "↓        full text · ↑ ↓ pages",
    "L        language · h  this help",
    "s        settings · w  technical field",
    "esc      go back",
};

constexpr const char* kAmbientBodyFr[] = {
    "Le lieu est lu par un scan WiFi passif,",
    "jamais connecté. Il teinte le tirage :",
    "yang ± 5 %, mutations + 4 % au plus.",
};
constexpr const char* kAmbientBodyEn[] = {
    "The place is read by a passive WiFi",
    "scan, never connected. It tints the",
    "casting: yang ± 5%, changes + 4% max.",
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
    "s · réglages",
    "w · champ",
    "tirer la ligne %d / 6",
    "lire la figure",
    "L'app complète",
    "du Yi Jing",
    "haut",
    "bas",
    "lecture du lieu…",
    "lecture du lieu · technique",
    "lecture du lieu coupée (réglages : s)",
    "lecture du lieu",
    "sons",
    "activée",
    "coupée",
    "activés",
    "coupés",
    kAmbientBodyFr,
    "glose",
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
    "s · settings",
    "w · field",
    "cast line %d / 6",
    "read the figure",
    "The complete",
    "Yi Jing app",
    "upper",
    "lower",
    "reading the place…",
    "reading the place · technical",
    "place reading is off (settings: s)",
    "reading the place",
    "sounds",
    "on",
    "off",
    "on",
    "off",
    kAmbientBodyEn,
    "gloss",
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
    // Revenir au tirage depuis une vue superposée (technique, aide,
    // réglages) ne doit jamais réinitialiser le tirage en cours.
    const bool overlayReturn = previous == Screen::kFieldInfo ||
                               previous == Screen::kHelp ||
                               previous == Screen::kSettings;
    if (s == Screen::kCasting && !overlayReturn) {
        session_.reset();
        session_.setWeights(0.f, 0.f);
        revealIndex_ = -1;
        completeAtMs_ = 0;
        particleCount_ = 0;
        scanning_ = false;
        if (ambientEnabled_) {
            scanning_ = true;
            scanStartMs_ = now;
            ambient_.startScan(now);
        }
    }
    // Ne pas revenir en page 0 quand on rentre de la lecture détaillée
    // ni de l'aide.
    if (s == Screen::kResult && previous != Screen::kDetail &&
        previous != Screen::kHelp && previous != Screen::kPromo) {
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
    if (k == Key::kField) {
        if (screen_ == Screen::kFieldInfo) {
            switchTo(fieldReturn_, now);
        } else if ((screen_ == Screen::kCasting && !scanning_) ||
                   screen_ == Screen::kResult) {
            fieldReturn_ = screen_;
            switchTo(Screen::kFieldInfo, now);
            playSound(Sound::kTick);
        }
        return;
    }
    if (k == Key::kSettings) {
        if (screen_ == Screen::kSettings) {
            switchTo(settingsReturn_, now);
        } else if (screen_ != Screen::kCasting || !scanning_) {
            settingsReturn_ = screen_ == Screen::kHelp ? helpReturn_ : screen_;
            settingsRow_ = 0;
            switchTo(Screen::kSettings, now);
        }
        return;
    }
    if (k == Key::kReset) {
        // esc contextuel : l'aide revient d'où l'on vient, la lecture
        // détaillée au résultat, partout ailleurs à l'accueil.
        if (screen_ == Screen::kHelp) {
            switchTo(helpReturn_, now);
        } else if (screen_ == Screen::kSettings) {
            switchTo(settingsReturn_, now);
        } else if (screen_ == Screen::kFieldInfo) {
            switchTo(fieldReturn_, now);
        } else if (screen_ == Screen::kDetail || screen_ == Screen::kStructure ||
                   screen_ == Screen::kPromo) {
            // esc recule d'un pas dans le parcours : la promo revient au
            // résultat, et c'est le résultat qui referme le cycle.
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
            // Le lieu se lit d'abord ; puis rythme : un trait à la fois.
            if (scanning_) break;
            if (revealIndex_ >= 0 && !reveal_.done(now)) break;
            if (session_.isComplete()) {
                playSound(Sound::kComplete);
                switchTo(Screen::kResult, now);
                break;
            }
            session_.castNextLine();
            revealIndex_ = static_cast<int>(session_.lineCount()) - 1;
            reveal_.start(now, kRevealMs);
            playSound(static_cast<Sound>(static_cast<int>(Sound::kReveal1) + revealIndex_));
            if (isMoving(session_.line(static_cast<size_t>(revealIndex_)))) {
                playSound(Sound::kMutation);
            }
            if (session_.isComplete()) completeAtMs_ = now + kRevealMs;
            break;
        }

        case Screen::kResult:
            if (k == Key::kNext || k == Key::kPrev) {
                resultPage_ = resultPage_ == 0 ? 1 : 0;
                enter_.start(now, 180);
                playSound(Sound::kTick);
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

        case Screen::kSettings:
            if (k == Key::kUp || k == Key::kDown) {
                settingsRow_ = settingsRow_ == 0 ? 1 : 0;
                playSound(Sound::kTick);
            } else if (k == Key::kAction) {
                if (settingsRow_ == 0) {
                    ambientEnabled_ = !ambientEnabled_;
                    ambient_.saveEnabled(ambientEnabled_);
                } else {
                    soundEnabled_ = !soundEnabled_;
                    ambient_.saveSoundEnabled(soundEnabled_);
                }
                playSound(Sound::kTick);
            }
            break;
    }
}

void App::playSound(Sound snd) {
    if (soundEnabled_ && sound_.play != nullptr) sound_.play(snd);
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
        case Screen::kSettings: drawSettings(now); break;
        case Screen::kFieldInfo: drawFieldInfo(now); break;
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

    // Réglages : coin bas-gauche, en écho aux coins hauts.
    s.setFont(&fonts::efontJA_12);
    s.setTextDatum(textdatum_t::bottom_left);
    s.setTextColor(fade(anim::lerp565(MINI_BG, MINI_TEXT_META, 0.7f), now), p.bg);
    s.drawString(t.settingsHint, MINI_MARGIN_X, MINI_H - MINI_BAND_PAD_Y);
    s.setTextDatum(textdatum_t::top_left);

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

void App::updateScan(uint32_t now) {
    if (!scanning_) return;
    const int n = ambient_.pollScan(now, aps_, kMaxAps);
    if (n < 0) return;
    // Le rituel dure au moins 1,4 s, même si le scan est plus prompt.
    if (now - scanStartMs_ < 1400) {
        apCount_ = n;  // résultats gardés, rideau maintenu
        return;
    }
    apCount_ = n < 0 ? 0 : n;
    field_ = readField(aps_, static_cast<size_t>(apCount_));
    const CastWeights w = weightsFromField(field_);
    session_.setWeights(w.yangBias, w.agingProb);
    // Une particule par réseau : position déduite du canal et de l'index,
    // respiration déphasée par le nombre d'or.
    particleCount_ = apCount_ > kMaxAps ? kMaxAps : apCount_;
    for (int i = 0; i < particleCount_; ++i) {
        Particle& pt = particles_[i];
        const float ch01 = static_cast<float>(aps_[i].channel % 14) / 13.f;
        pt.baseX = 14.f + ch01 * (MINI_W - 28.f) +
                   static_cast<float>((i * 37) % 11) - 5.f;
        pt.baseY = 16.f + static_cast<float>((i * 53) % (MINI_H - 40));
        pt.phase = static_cast<float>(i) * 2.399963f;
        pt.ampX = 5.f + static_cast<float>((i * 29) % 7);
        pt.ampY = 3.f + static_cast<float>((i * 17) % 5);
        pt.lum = 0.25f + 0.75f * ((static_cast<float>(aps_[i].rssi) + 90.f) / 50.f);
        if (pt.lum > 1.f) pt.lum = 1.f;
        if (pt.lum < 0.25f) pt.lum = 0.25f;
    }
    scanning_ = false;
    playSound(Sound::kChime);
}

void App::drawAmbientField(uint32_t now) {
    if (particleCount_ == 0) return;
    auto& s = frame_;
    const float t = static_cast<float>(now) * 0.001f;

    // Le champ répond au tirage : bref surcroît de lumière à chaque trait.
    float response = 0.f;
    if (revealIndex_ >= 0 && !reveal_.done(now)) {
        response = 0.7f * (1.f - reveal_.progress(now));
    }

    // Une onde concentrique à la fois, émise tour à tour par les réseaux.
    constexpr uint32_t kRipplePeriodMs = 2600;
    const int rippleIdx =
        static_cast<int>((now / kRipplePeriodMs) % static_cast<uint32_t>(particleCount_));
    const float rippleAge =
        static_cast<float>(now % kRipplePeriodMs) / static_cast<float>(kRipplePeriodMs);

    for (int i = 0; i < particleCount_; ++i) {
        const Particle& pt = particles_[i];
        const float x = pt.baseX + std::sin(t * 0.45f + pt.phase) * pt.ampX;
        const float y = pt.baseY + std::sin(t * 0.31f + pt.phase * 1.7f) * pt.ampY;
        const int xi = static_cast<int>(x), yi = static_cast<int>(y);
        // Respiration propre à chaque particule, jamais à zéro.
        const float breath = 0.5f + 0.5f * (0.5f + 0.5f * std::sin(t * 0.9f + pt.phase));
        const float lum =
            anim::clamp01((0.12f + pt.lum * breath * 0.8f) * (1.f + response));

        // Lueur : halo doux sous un cœur net.
        const uint16_t halo =
            fade(anim::lerp565(MINI_BG, MINI_TEXT_META, lum * 0.30f), now);
        const uint16_t core = fade(anim::lerp565(MINI_BG, MINI_TEXT_META, lum), now);
        s.fillRect(xi - 1, yi - 1, 4, 4, halo);
        s.fillRect(xi, yi, 2, 2, core);

        // L'onde du réseau élu : deux cercles concentriques qui s'éteignent.
        if (i == rippleIdx) {
            const float r = 4.f + rippleAge * 18.f;
            const float fadeOut = (1.f - rippleAge) * (1.f - rippleAge);
            const float rl = anim::clamp01(pt.lum * fadeOut * (0.5f + response));
            const uint16_t rc =
                fade(anim::lerp565(MINI_BG, MINI_TEXT_META, rl * 0.55f), now);
            const int ri = static_cast<int>(r);
            s.drawCircle(xi + 1, yi + 1, ri, rc);
            s.drawCircle(xi + 1, yi + 1, ri - 1, rc);
        }
    }
}

void App::drawCasting(uint32_t now) {
    auto& s = frame_;
    const ui::Palette p = palette(now);
    const Strings& t = str(lang_);

    updateScan(now);
    drawAmbientField(now);

    if (scanning_) {
        // La lecture du lieu : le champ n'est pas encore révélé.
        const float breath = 0.4f + 0.6f * anim::pulse(now, 1600);
        ui::Palette pp = p;
        pp.meta = fade(anim::lerp565(MINI_BG, MINI_TEXT_META, breath), now);
        const ui::PromptSeg segs[] = {{nullptr, t.scanText}};
        ui::drawPrompt(s, pp, lang_, kCastingPromptY, segs, 1);
        // Les emplacements des six traits attendent déjà.
        ui::HexLine none[6] = {};
        ui::drawHexagram(s, p, kCastingX, kCastingY, none, 0, ui::YAO_L, false);
        return;
    }

    ui::HexLine lines[6];
    sessionLines(session_, false, lines);
    const int revealed = static_cast<int>(session_.lineCount());
    const float lastReveal = (revealIndex_ >= 0) ? reveal_.progress(now) : 1.f;
    ui::drawHexagram(s, p, kCastingX, kCastingY, lines, revealed, ui::YAO_L, true,
                     lastReveal);

    // Vue technique du champ : coin bas-droite, discret.
    s.setFont(&fonts::efontJA_12);
    s.setTextDatum(textdatum_t::bottom_right);
    s.setTextColor(fade(anim::lerp565(MINI_BG, MINI_TEXT_META, 0.7f), now), p.bg);
    s.drawString(t.fieldHint, MINI_W - MINI_MARGIN_X, MINI_H - MINI_BAND_PAD_Y);
    s.setTextDatum(textdatum_t::top_left);

    char text[48];
    if (session_.isComplete()) {
        snprintf(text, sizeof(text), "%s", t.castDoneText);
        if (completeAtMs_ != 0 && now > completeAtMs_ + kAutoResultDelayMs) {
            playSound(Sound::kComplete);
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
    const HexagramInfo& h = hexagram(detailNumber_);
    const Strings& t = str(lang_);
    const bool fr = lang_ == Lang::kFr;

    // Si la glose du résultat a été tronquée par clampGloss (plus de deux
    // lignes de colonne pleine), la lecture s'ouvre sur son texte entier.
    const char* desc = description(h, lang_);
    if (wrapLines(frame_, desc, MINI_COL_W).size() > 2) {
        appendDocText(t.detailGloss, true);
        appendDocText(desc, false);
    }

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
    for (int i = 0; i < 7; ++i) {
        const HelpSection& sec = t.helpSections[i];
        appendDocText(sec.title, true);
        if (sec.body != nullptr) {
            appendDocText(sec.body, false);
        } else {
            // Dernière section : la liste des touches, une par ligne.
            for (int j = 0; j < 6; ++j) appendDocText(t.helpKeys[j], false);
            break;
        }
    }
}

void App::drawSettings(uint32_t now) {
    auto& s = frame_;
    const ui::Palette p = palette(now);
    const Strings& t = str(lang_);

    ui::drawDocHeader(s, p, keyLabel("settings", lang_), 1, 1);

    struct Row {
        const char* name;
        bool on;
    };
    struct RowState {
        const char* on;
        const char* off;
    };
    const Row rows[2] = {{t.rowAmbient, ambientEnabled_}, {t.rowSound, soundEnabled_}};
    const RowState states[2] = {{t.stateOnAmbient, t.stateOffAmbient},
                                {t.stateOnSound, t.stateOffSound}};
    s.setFont(&fonts::efontJA_12);
    for (int i = 0; i < 2; ++i) {
        const int y = 28 + i * (MINI_BODY_LEAD + SP_4);
        const bool sel = settingsRow_ == i;
        s.setTextDatum(textdatum_t::top_left);
        s.setTextColor(sel ? p.text : p.meta, p.bg);
        char name[48];
        snprintf(name, sizeof(name), "%s%s", sel ? "· " : "  ", rows[i].name);
        s.drawString(name, MINI_MARGIN_X, y);
        s.setTextDatum(textdatum_t::top_right);
        s.setTextColor(rows[i].on ? p.text : p.meta, p.bg);
        s.drawString(rows[i].on ? states[i].on : states[i].off, MINI_W - MINI_MARGIN_X, y);
    }

    // Explication honnête de la lecture du lieu.
    s.setTextDatum(textdatum_t::top_left);
    s.setTextColor(p.meta, p.bg);
    for (int i = 0; i < 3; ++i) {
        s.drawString(t.ambientBody[i], MINI_MARGIN_X, 72 + i * MINI_BODY_LEAD);
    }

    ui::drawFooterBar(s, p, lang_, ui::kFooterSpecs[ui::kFooterSettings]);
}

// Vue technique de la lecture du lieu : réseaux, valeurs, pondérations et
// probabilités résultantes. La transparence est la condition de la règle.
void App::drawFieldInfo(uint32_t now) {
    auto& s = frame_;
    const ui::Palette p = palette(now);
    const Strings& t = str(lang_);

    ui::drawDocHeader(s, p, t.fieldTitle, 1, 1);
    s.setFont(&fonts::efontJA_12);
    s.setTextDatum(textdatum_t::top_left);
    char buf[64];
    int y = 26;

    if (!ambientEnabled_) {
        s.setTextColor(p.meta, p.bg);
        s.drawString(t.fieldOffText, MINI_MARGIN_X, y);
    } else {
        // Réseaux et canaux (jamais de SSID : ils ne sont pas collectés).
        s.setTextColor(p.text, p.bg);
        snprintf(buf, sizeof(buf), lang_ == Lang::kFr ? "%d réseaux" : "%d networks",
                 apCount_);
        s.drawString(buf, MINI_MARGIN_X, y);
        y += MINI_BODY_LEAD;
        s.setTextColor(p.meta, p.bg);
        char list[64] = {0};
        int used = 0;
        for (int i = 0; i < apCount_ && i < 9; ++i) {
            used += snprintf(list + used, sizeof(list) - used, "%s%d",
                             i ? " " : "", aps_[i].channel);
        }
        snprintf(buf, sizeof(buf), "ch   %s", apCount_ ? list : "—");
        s.drawString(buf, MINI_MARGIN_X, y);
        y += MINI_BODY_LEAD;
        used = 0;
        list[0] = 0;
        for (int i = 0; i < apCount_ && i < 6; ++i) {
            used += snprintf(list + used, sizeof(list) - used, "%s%d",
                             i ? " " : "", aps_[i].rssi);
        }
        snprintf(buf, sizeof(buf), "dBm  %s", apCount_ ? list : "—");
        s.drawString(buf, MINI_MARGIN_X, y);
        y += MINI_BODY_LEAD;

        snprintf(buf, sizeof(buf),
                 lang_ == Lang::kFr ? "densité %.2f · agitation %.2f"
                                    : "density %.2f · agitation %.2f",
                 static_cast<double>(field_.density01),
                 static_cast<double>(field_.agitation01));
        s.drawString(buf, MINI_MARGIN_X, y);
        y += MINI_BODY_LEAD;
        snprintf(buf, sizeof(buf), "β yang %+.3f · γ %.3f",
                 static_cast<double>(session_.yangBias()),
                 static_cast<double>(session_.agingProb()));
        s.drawString(buf, MINI_MARGIN_X, y);
        y += MINI_BODY_LEAD;
        const double py = 0.5 + static_cast<double>(session_.yangBias());
        const double po = 0.25 + static_cast<double>(session_.agingProb());
        snprintf(buf, sizeof(buf), "p(6·7·8·9) %.1f %.1f %.1f %.1f %%",
                 (1 - py) * po * 100, py * (1 - po) * 100, (1 - py) * (1 - po) * 100,
                 py * po * 100);
        s.setTextColor(p.text, p.bg);
        s.drawString(buf, MINI_MARGIN_X, y);
    }

    ui::drawFooterBar(s, p, lang_, ui::kFooterSpecs[ui::kFooterField]);
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
