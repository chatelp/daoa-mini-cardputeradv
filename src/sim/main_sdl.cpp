// Entrée simulateur macOS : LovyanGFX + SDL2.
// Fenêtre 240 × 135 zoomée ×3. Mode `--shot <dossier>` : rejoue un tirage
// déterministe et écrit des captures BMP (vérification visuelle sans écran).
#include <lgfx/v1/platforms/sdl/Panel_sdl.hpp>
#if defined(SDL_h_)

#define LGFX_AUTODETECT
#include <LGFX_AUTODETECT.hpp>
#include <LovyanGFX.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include "../ui/app.h"
#include "../ui/theme.h"

namespace {

constexpr uint32_t kShotSeed = 20260808u;

uint32_t g_seed = 0;
bool g_shotMode = false;
bool g_fontTest = false;
bool g_framesMode = false;
std::string g_shotDir;

// Hôte ambiant simulé : un scan de ~1,4 s qui « découvre » des réseaux
// plausibles, déterministes par graine. Sons : muets sur Mac.
uint32_t g_scanStart = 0;
bool g_scanPending = false;

void simStartScan(uint32_t nowMs) {
    g_scanStart = nowMs;
    g_scanPending = true;
}

int simPollScan(uint32_t nowMs, daoa::ApReading* out, int max) {
    if (!g_scanPending || nowMs - g_scanStart < 1350) return -1;
    g_scanPending = false;
    uint32_t st = (g_seed ? g_seed : 1u) ^ 0xA5A5A5A5u;
    auto nx = [&st]() {
        st ^= st << 13;
        st ^= st >> 17;
        st ^= st << 5;
        return st;
    };
    const int n = 3 + static_cast<int>(nx() % 9);  // 3..11 réseaux
    const int count = n < max ? n : max;
    for (int i = 0; i < count; ++i) {
        out[i].rssi = static_cast<int8_t>(-40 - static_cast<int>(nx() % 50));
        out[i].channel = static_cast<uint8_t>(1 + (nx() % 13));
    }
    return count;
}

bool g_ambientEnabled = true;
bool g_soundEnabled = true;
uint8_t g_volumeLevel = 1;
bool simLoadEnabled() { return g_ambientEnabled; }
void simSaveEnabled(bool e) { g_ambientEnabled = e; }
bool simLoadSound() { return g_soundEnabled; }
void simSaveSound(bool e) { g_soundEnabled = e; }
uint8_t simLoadVolume() { return g_volumeLevel; }
void simSaveVolume(uint8_t v) { g_volumeLevel = v; }

const daoa::AmbientHost kSimAmbientHost{simStartScan,  simPollScan,   simLoadEnabled,
                                        simSaveEnabled, simLoadSound, simSaveSound,
                                        simLoadVolume,  simSaveVolume};
const daoa::SoundHost kSimSoundHost{nullptr, nullptr};

// PRNG xorshift déterministe pour le simulateur (l'appareil, lui, utilise
// le TRNG matériel — voir src/main.cpp).
uint32_t simRand() {
    static uint32_t state = g_seed ? g_seed : 0x9E3779B9u;
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

bool writeBmp(lgfx::LGFX_Sprite& s, const std::string& path) {
    const int w = s.width(), h = s.height();
    std::vector<uint16_t> px(static_cast<size_t>(w) * h);
    s.readRect(0, 0, w, h, px.data());

    const int rowBytes = ((w * 3) + 3) & ~3;
    const uint32_t dataSize = static_cast<uint32_t>(rowBytes) * h;
    const uint32_t fileSize = 54 + dataSize;

    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    uint8_t hdr[54] = {0};
    hdr[0] = 'B'; hdr[1] = 'M';
    memcpy(hdr + 2, &fileSize, 4);
    const uint32_t offset = 54, dibSize = 40;
    memcpy(hdr + 10, &offset, 4);
    memcpy(hdr + 14, &dibSize, 4);
    memcpy(hdr + 18, &w, 4);
    memcpy(hdr + 22, &h, 4);
    hdr[26] = 1;       // planes
    hdr[28] = 24;      // bpp
    memcpy(hdr + 34, &dataSize, 4);
    fwrite(hdr, 1, 54, f);

    std::vector<uint8_t> row(static_cast<size_t>(rowBytes), 0);
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            // readRect renvoie du RGB565 avec octets inversés (ordre bus).
            uint16_t v = px[static_cast<size_t>(y) * w + x];
            v = static_cast<uint16_t>((v >> 8) | (v << 8));
            const uint8_t r5 = (v >> 11) & 0x1F, g6 = (v >> 5) & 0x3F, b5 = v & 0x1F;
            row[x * 3 + 0] = static_cast<uint8_t>((b5 << 3) | (b5 >> 2));
            row[x * 3 + 1] = static_cast<uint8_t>((g6 << 2) | (g6 >> 4));
            row[x * 3 + 2] = static_cast<uint8_t>((r5 << 3) | (r5 >> 2));
        }
        fwrite(row.data(), 1, row.size(), f);
    }
    fclose(f);
    return true;
}

void shot(daoa::App& app, uint32_t t, const char* name) {
    app.renderFrame(t);
    const std::string path = g_shotDir + "/" + name;
    if (writeBmp(app.frame(), path)) {
        printf("capture : %s\n", path.c_str());
    } else {
        printf("ÉCHEC capture : %s\n", path.c_str());
    }
}

// Rejoue un tirage complet avec un aléa déterministe et capture chaque écran.
int runShots() {
    daoa::App app(simRand, kSimAmbientHost, kSimSoundHost);
    using daoa::Key;

    shot(app, 1400, "01_accueil.bmp");

    app.handleKey(Key::kAction, 1500);                      // → tirage, scan lancé
    shot(app, 2300, "02_lecture_du_lieu.bmp");              // le lieu se lit
    app.renderFrame(3000);                                  // scan posé, champ né

    app.handleKey(Key::kAction, 3100);
    app.handleKey(Key::kAction, 3700);
    app.handleKey(Key::kAction, 4300);
    shot(app, 4600, "03_tirage_revelation.bmp");            // 3 traits + champ

    app.handleKey(Key::kAction, 5200);
    app.handleKey(Key::kAction, 5800);
    app.handleKey(Key::kAction, 6400);                      // 6e trait
    shot(app, 6700, "04_tirage_complet.bmp");

    app.handleKey(Key::kField, 6800);                       // vue technique
    shot(app, 7200, "05_champ_technique_en.bmp");
    app.handleKey(Key::kField, 7300);                       // retour tirage

    app.renderFrame(8000);                                  // bascule auto
    shot(app, 8400, "06_resultat_en.bmp");

    app.handleKey(Key::kDown, 8500);                        // → lecture détaillée
    shot(app, 8800, "07_texte_p1_en.bmp");
    app.handleKey(Key::kReset, 8900);                       // esc → résultat

    app.handleKey(Key::kNext, 9000);
    shot(app, 9300, "08_complementaire_en.bmp");

    app.handleKey(Key::kLang, 9400);
    app.handleKey(Key::kNext, 9450);                        // retour page 1
    shot(app, 9800, "09_resultat_fr.bmp");

    app.handleKey(Key::kDown, 9850);                        // détail FR
    shot(app, 10200, "10_texte_p1_fr.bmp");
    for (uint32_t t = 10210; t <= 10800; t += 40) {
        app.handleKey(Key::kDown, t);                       // jusqu'à structure
    }
    shot(app, 10900, "11_structure_fr.bmp");
    app.handleKey(Key::kLang, 10950);
    shot(app, 11300, "12_structure_en.bmp");
    app.handleKey(Key::kLang, 11350);
    app.handleKey(Key::kReset, 11400);                      // esc → résultat

    app.handleKey(Key::kSettings, 11500);                   // réglages
    shot(app, 11900, "13_reglages_fr.bmp");
    app.handleKey(Key::kDown, 11950);                       // ligne sons
    app.handleKey(Key::kDown, 11970);                       // ligne volume
    app.handleKey(Key::kAction, 12000);                     // niveau suivant
    shot(app, 12400, "14_reglages_volume_fr.bmp");
    app.handleKey(Key::kReset, 12100);                      // esc → résultat

    app.handleKey(Key::kLang, 12150);                       // → EN
    app.handleKey(Key::kAction, 12200);                     // → promo
    shot(app, 12600, "15_promo.bmp");

    app.handleKey(Key::kLang, 12650);                       // → FR
    app.handleKey(Key::kHelp, 12700);                       // aide (FR ici)
    shot(app, 13100, "16_aide_p1_fr.bmp");
    for (uint32_t t = 13150; t <= 13500; t += 50) {
        app.handleKey(Key::kDown, t);                       // jusqu'en bas
    }
    shot(app, 13700, "17_aide_touches_fr.bmp");

    return 0;
}

// Suite de frames du champ ambiant pendant un tirage, pour en faire un GIF :
// le champ ne se juge qu'en mouvement.
int runFrames() {
    daoa::App app(simRand, kSimAmbientHost, kSimSoundHost);
    using daoa::Key;
    uint32_t t = 1000;
    app.handleKey(Key::kAction, t);          // → tirage, scan lancé
    t = 2600;                                 // scan posé
    app.renderFrame(t);
    app.handleKey(Key::kAction, t);           // trait 1
    t += 700;
    app.handleKey(Key::kAction, t);           // trait 2
    t += 700;
    app.handleKey(Key::kAction, t);           // trait 3

    char name[32];
    // 60 frames à 66 ms ≈ 4 s de champ vivant, un trait posé en cours de route.
    for (int i = 0; i < 60; ++i) {
        const uint32_t frameT = t + static_cast<uint32_t>(i) * 66;
        if (i == 24) app.handleKey(Key::kAction, frameT);  // trait 4 : le champ répond
        if (i == 48) app.handleKey(Key::kAction, frameT);  // trait 5
        app.renderFrame(frameT);
        snprintf(name, sizeof(name), "/f%03d.bmp", i);
        writeBmp(app.frame(), g_shotDir + name);
    }
    printf("60 frames dans %s\n", g_shotDir.c_str());
    return 0;
}

// Écran de contrôle typographique : couverture des accents FR par fonte.
int runFontTest() {
    lgfx::LGFX_Sprite s;
    s.setColorDepth(16);
    if (s.createSprite(240, 135) == nullptr) return 1;
    s.fillSprite(daoa::theme::kBg);
    s.setTextColor(daoa::theme::kFg, daoa::theme::kBg);
    s.setTextDatum(textdatum_t::top_left);
    const char* sample = "ÉÈÊÀÇÙ Œœ éèêàçù";
    s.setFont(&fonts::efontCN_12);
    s.drawString("CN12", 4, 4);
    s.drawString(sample, 50, 4);
    s.setFont(&fonts::efontCN_16_b);
    s.drawString("CN16b", 4, 22);
    s.drawString(sample, 50, 22);
    s.setFont(&fonts::efontJA_12);
    s.drawString("JA12", 4, 46);
    s.drawString(sample, 50, 46);
    s.setFont(&fonts::efontJA_16_b);
    s.drawString("JA16b", 4, 64);
    s.drawString(sample, 50, 64);
    // Les 71 caractères distincts du corpus, couverture efontCN taille 12.
    s.setFont(&fonts::efontCN_12);
    s.drawString("乾坤屯蒙需讼师比小畜履泰否同人大有谦豫", 4, 84);
    s.drawString("随蛊临观噬嗑贲剥复无妄颐过坎离咸恒遯壮", 4, 97);
    s.drawString("晋明夷家睽蹇解损益夬姤萃升困井革鼎震艮", 4, 110);
    s.drawString("渐归妹丰旅巽兑涣节中孚既济未", 4, 123);
    // Les trois manquants d'efontCN, tentés en efontJA.
    s.setFont(&fonts::efontJA_16);
    s.drawString("遯夬姤", 190, 40);
    s.setFont(&fonts::efontJA_12);
    printf("larg. '…'=%d  '.'=%d  'a'=%d px\n", s.textWidth("…"), s.textWidth("."), s.textWidth("a"));
    daoa::App tmp(simRand, kSimAmbientHost, kSimSoundHost);  // parité de liens
    (void)tmp;
    writeBmp(s, g_shotDir + "/00_fonttest.bmp");
    printf("capture : %s/00_fonttest.bmp\n", g_shotDir.c_str());
    return 0;
}

int runInteractive(bool* running) {
    static LGFX lcd(daoa::theme::kWidth, daoa::theme::kHeight, 3);
    lcd.init();

    daoa::App app(simRand, kSimAmbientHost, kSimSoundHost);
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    Uint8 prev[SDL_NUM_SCANCODES] = {0};

    const struct { SDL_Scancode sc; daoa::Key key; } kMap[] = {
        {SDL_SCANCODE_SPACE, daoa::Key::kAction},
        {SDL_SCANCODE_RETURN, daoa::Key::kAction},
        {SDL_SCANCODE_LEFT, daoa::Key::kPrev},
        {SDL_SCANCODE_RIGHT, daoa::Key::kNext},
        {SDL_SCANCODE_UP, daoa::Key::kUp},
        {SDL_SCANCODE_DOWN, daoa::Key::kDown},
        {SDL_SCANCODE_L, daoa::Key::kLang},
        {SDL_SCANCODE_H, daoa::Key::kHelp},
        {SDL_SCANCODE_S, daoa::Key::kSettings},
        {SDL_SCANCODE_W, daoa::Key::kField},
        {SDL_SCANCODE_ESCAPE, daoa::Key::kReset},
    };

    while (*running) {
        const uint32_t now = SDL_GetTicks();
        for (const auto& m : kMap) {
            if (keys[m.sc] && !prev[m.sc]) app.handleKey(m.key, now);
            prev[m.sc] = keys[m.sc];
        }
        app.render(lcd, now);
        SDL_Delay(16);
    }
    return 0;
}

int userMain(bool* running) { return runInteractive(running); }

}  // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--shot") == 0 && i + 1 < argc) {
            g_shotMode = true;
            g_shotDir = argv[++i];
            g_seed = kShotSeed;
        } else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            g_framesMode = true;
            g_shotDir = argv[++i];
            g_seed = kShotSeed;
        } else if (strcmp(argv[i], "--fonttest") == 0 && i + 1 < argc) {
            g_shotDir = argv[++i];
            g_seed = kShotSeed;
            g_fontTest = true;
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = static_cast<uint32_t>(strtoul(argv[++i], nullptr, 0));
        }
    }
    if (!g_seed) g_seed = static_cast<uint32_t>(time(nullptr));
    // Le mode capture dessine dans un sprite hors écran : pas besoin de la
    // boucle d'événements SDL (qui ne rend jamais la main).
    if (g_fontTest) return runFontTest();
    if (g_framesMode) return runFrames();
    if (g_shotMode) return runShots();
    return lgfx::Panel_sdl::main(userMain);
}

#endif  // SDL_h_
