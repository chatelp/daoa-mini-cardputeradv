// Machine à états des écrans + rendu. Ne dépend que de lgfx:: (jamais du
// matériel) : le même code dessine sur l'appareil et dans le simulateur SDL.
// Les écrans composent les gabarits du design system daoa-mini-design via
// les composants de components.h ; aucune cote en dur hors daoa_tokens.h.
#pragma once

#include <cstdint>
#include <vector>

#include "../ambient/ambient_host.h"
#include "../anim/easing.h"
#include "../content/content.h"
#include "../core/ambient.h"
#include "../core/casting.h"
#include "../sound/ui_sound.h"
#include "components.h"
#include "gfx.h"

namespace daoa {

// Entrées abstraites ; le mapping clavier vit dans main.cpp / main_sdl.cpp.
enum class Key : uint8_t {
    kNone, kAction, kPrev, kNext, kUp, kDown, kLang, kHelp, kSettings, kField, kReset
};

class App {
  public:
    App(RandomFn rand32, const AmbientHost& ambient, const SoundHost& sound)
        : session_(rand32),
          ambient_(ambient),
          sound_(sound),
          ambientEnabled_(ambient.loadEnabled()),
          soundEnabled_(ambient.loadSoundEnabled()),
          volumeLevel_(ambient.loadVolumeLevel()) {}

    void handleKey(Key k, uint32_t nowMs);

    // Dessine l'image dans le sprite interne (sans l'afficher).
    void renderFrame(uint32_t nowMs);

    // Dessine puis pousse vers l'écran.
    void render(lgfx::LGFX_Device& dev, uint32_t nowMs);

    // Accès au dernier rendu (captures du simulateur).
    lgfx::LGFX_Sprite& frame() { return frame_; }

  private:
    enum class Screen : uint8_t {
        kWelcome, kCasting, kResult, kDetail, kStructure, kPromo, kHelp, kSettings,
        kFieldInfo
    };

    // Une particule du champ ambiant : un réseau perçu, qui dérive et respire.
    struct Particle {
        float baseX, baseY;
        float phase;
        float ampX, ampY;
        float lum;  // énergie du signal, 0..1
    };

    void switchTo(Screen s, uint32_t now);
    bool ensureFrame();
    uint16_t fade(uint16_t c, uint32_t now) const;
    ui::Palette palette(uint32_t now) const;

    void drawWelcome(uint32_t now);
    void drawCasting(uint32_t now);
    void drawResult(uint32_t now);
    void drawDetail(uint32_t now);
    void drawStructure(uint32_t now);
    void drawPromo(uint32_t now);
    void drawHelp(uint32_t now);
    void drawSettings(uint32_t now);
    void drawFieldInfo(uint32_t now);
    void drawAmbientField(uint32_t now);
    void updateScan(uint32_t now);
    void playSound(Sound s);
    void buildDetailDoc();
    void buildHelpDoc();
    void appendDocText(const char* text, bool meta);
    // Rendu commun des écrans-documents (lecture détaillée, aide).
    void drawDocScreen(const char* context, ui::FooterId footer, uint32_t now);
    int docPageCount() const;

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

    // Lecture du lieu et sons.
    const AmbientHost& ambient_;
    const SoundHost& sound_;
    bool ambientEnabled_;
    bool soundEnabled_;
    uint8_t volumeLevel_;  // 0 doux · 1 moyen · 2 fort
    bool scanning_ = false;
    uint32_t scanStartMs_ = 0;
    ApReading aps_[kMaxAps];
    int apCount_ = 0;
    AmbientField field_;
    Particle particles_[kMaxAps];
    int particleCount_ = 0;
    Screen settingsReturn_ = Screen::kWelcome;
    int settingsRow_ = 0;
    static constexpr int kSettingsRows = 3;
    Screen fieldReturn_ = Screen::kCasting;

    lgfx::LGFX_Sprite frame_;
    bool frameReady_ = false;
};

}  // namespace daoa
