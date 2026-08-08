// Point d'inclusion unique du moteur graphique.
// Le code UI ne dépend que de l'API lgfx:: (commune aux deux libs) :
//   - appareil : M5GFX (via M5Cardputer)
//   - simulateur macOS : LovyanGFX + Panel_sdl
#pragma once

#if defined(DAOA_SIM)
#define LGFX_USE_V1
#include <LovyanGFX.h>
#else
#include <M5GFX.h>
#endif
