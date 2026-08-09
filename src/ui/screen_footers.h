// Pieds de page de chaque écran, par identifiants (HANDOFF-v2, tâche 2).
// Source unique : l'app compose ses barres depuis cette table, et le test
// natif vérifie que chaque variante tient en 38 colonnes dans toutes les
// langues (contrainte dure du design system).
#pragma once

#include "daoa_key_labels.h"

namespace daoa::ui {

struct FooterActionSpec {
    const char* key;    // identifiant ou touche sérigraphiée
    const char* label;  // identifiant, ou nul
};

struct FooterSpec {
    const char* name;      // pour les messages de test
    const char* legendId;  // identifiant, ou nul
    FooterActionSpec actions[3];
    int actionCount;
};

enum FooterId {
    kFooterResultDrawn = 0,
    kFooterResultOther,
    kFooterResultStable,
    kFooterDoc,
    kFooterHelp,
    kFooterStructure,
    kFooterCount,
};

inline constexpr FooterSpec kFooterSpecs[kFooterCount] = {
    {"resultat-obtenue", "drawn", {{"↓", "text"}, {"space", nullptr}, {"esc", nullptr}}, 3},
    {"resultat-complementaire", "other",
     {{"↓", "text"}, {"space", nullptr}, {"esc", nullptr}}, 3},
    {"resultat-stable", "stable", {{"↓", "text"}, {"space", nullptr}, {"esc", nullptr}}, 3},
    {"lecture", nullptr, {{"↑↓", nullptr}, {"esc", nullptr}, {nullptr, nullptr}}, 2},
    {"aide", nullptr, {{"esc", "home"}, {nullptr, nullptr}, {nullptr, nullptr}}, 1},
    {"structure", "structure", {{"↑↓", nullptr}, {"esc", nullptr}, {nullptr, nullptr}}, 2},
};

}  // namespace daoa::ui
