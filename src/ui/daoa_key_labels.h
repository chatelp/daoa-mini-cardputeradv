// Table des libellés de touche (HANDOFF-v2, tâche 2) — port de
// components/chrome/keyLabels.js. Les écrans ne portent JAMAIS de chaîne en
// dur : ils passent un identifiant, le rendu résout selon la langue.
// C++ pur, sans dépendance graphique : testé en natif.
#pragma once

#include <cstring>

#include "../content/content.h"

namespace daoa {

struct KeyLabelEntry {
    const char* id;
    const char* fr;
    const char* en;
};

// Mêmes identifiants que keyLabels.js. « stable » est une extension firmware
// (légende du résultat sans mutation, absente de la table JS) — écart signalé
// dans le retour v2.
inline constexpr KeyLabelEntry kKeyLabels[] = {
    {"space", "espace", "space"},            // 6
    {"esc", "esc", "esc"},                   // 3
    {"help", "aide", "help"},                // 4
    {"language", "langue", "language"},      // 8
    {"begin", "commencer", "begin"},         // 9
    {"text", "texte", "text"},               // 5
    {"home", "accueil", "home"},             // 7
    {"back", "retour", "back"},              // 6
    {"figure", "figure", "figure"},          // 6
    {"pages", "pages", "pages"},             // 5
    {"structure", "structure", "structure"}, // 9
    {"drawn", "figure obtenue", "first figure"},    // 14
    {"other", "complémentaire", "complementary"},   // 14
    {"stable", "figure stable", "stable figure"},   // 13 (extension)
};

// Résout un identifiant. Une chaîne inconnue de la table passe telle
// quelle — échappatoire pour les touches sérigraphiées (esc, L, h, ↑, ↓).
inline const char* keyLabel(const char* idOrText, Lang lang) {
    if (idOrText == nullptr) return "";
    for (const auto& e : kKeyLabels) {
        if (std::strcmp(e.id, idOrText) == 0) return lang == Lang::kFr ? e.fr : e.en;
    }
    return idOrText;
}

}  // namespace daoa
