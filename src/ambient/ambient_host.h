// Frontière matérielle de la lecture du lieu : le scan WiFi et la
// persistance du réglage. L'app ne voit que cette interface ; main.cpp
// (WiFi réel + NVS) et main_sdl.cpp (scan simulé) fournissent chacun la
// leur. Les fonctions prennent le temps de l'app (nowMs) pour rester
// pilotables par le simulateur et ses captures déterministes.
#pragma once

#include <cstdint>

#include "../core/ambient.h"

namespace daoa {

inline constexpr int kMaxAps = 24;

struct AmbientHost {
    // Lance un scan passif (asynchrone). Radio coupée sitôt le scan fini.
    void (*startScan)(uint32_t nowMs);
    // −1 tant que le scan court ; sinon nombre de réseaux copiés (≤ max).
    int (*pollScan)(uint32_t nowMs, ApReading* out, int max);
    // Réglage « lecture du lieu ». Défaut : activée (décision 2026-08-09).
    bool (*loadEnabled)();
    void (*saveEnabled)(bool enabled);
    // Réglage « sons ». Défaut : activés.
    bool (*loadSoundEnabled)();
    void (*saveSoundEnabled)(bool enabled);
};

}  // namespace daoa
