// Machine à états des écrans + rendu. Ne dépend que de lgfx:: (jamais du
// matériel) : le même code dessine sur l'appareil et dans le simulateur SDL.
// Les écrans composent les gabarits du design system daoa-mini-design via
// les composants de components.h ; aucune cote en dur hors daoa_tokens.h.
#pragma once

#include <cstdint>
#include <vector>

#include "../anim/easing.h"
#include "../content/content.h"
#include "../core/casting.h"
#include "components.h"
#include "gfx.h"

namespace daoa {

// Entrées abstraites ; le mapping clavier vit dans main.cpp / main_sdl.cpp.
enum class Key : uint8_t { kNone, kAction, kPrev, kNext, kUp, kDown, kLang, kHelp, kReset };

class App {
  public:
    explicit App(RandomFn rand32) : session_(rand32) {}

    void handleKey(Key k, uint32_t nowMs);

    // Dessine l'image dans le sprite interne (sans l'afficher).
    void renderFrame(uint32_t nowMs);

    // Dessine puis pousse vers l'écran.
    void render(lgfx::LGFX_Device& dev, uint32_t nowMs);

    // Accès au dernier rendu (captures du simulateur).
    lgfx::LGFX_Sprite& frame() { return frame_; }

  private:
    enum class Screen : uint8_t { kWelcome, kCasting, kResult, kDetail, kPromo, kHelp };

    void switchTo(Screen s, uint32_t now);
    bool ensureFrame();
    uint16_t fade(uint16_t c, uint32_t now) const;
    ui::Palette palette(uint32_t now) const;

    void drawWelcome(uint32_t now);
    void drawCasting(uint32_t now);
    void drawResult(uint32_t now);
    void drawDetail(uint32_t now);
    void drawPromo(uint32_t now);
    void drawHelp(uint32_t now);
    void buildDetailDoc();
    void buildHelpDoc();
    void appendDocText(const char* text, bool meta);
    // Rendu commun des écrans-documents (lecture détaillée, aide).
    void drawDocScreen(const char* context, const char* footerLegend, uint32_t now);

    CastSession session_;
    Screen screen_ = Screen::kWelcome;
    // Anglais par défaut (décision 2026-08-08) ; bascule L sur l'accueil.
    Lang lang_ = Lang::kEn;
    uint8_t resultPage_ = 0;  // 0 = figure obtenue, 1 = complémentaire

    anim::Timeline enter_;    // fondu d'entrée d'écran
    anim::Timeline reveal_;   // révélation de la dernière ligne tirée
    int revealIndex_ = -1;
    uint32_t completeAtMs_ = 0;

    // Lecture détaillée (textes du corpus Daoa 2) et aide (même moteur).
    uint8_t detailNumber_ = 1;
    std::vector<ui::ReaderLine> doc_;
    int docPage_ = 0;
    Screen helpReturn_ = Screen::kWelcome;

    lgfx::LGFX_Sprite frame_;
    bool frameReady_ = false;
};

}  // namespace daoa
