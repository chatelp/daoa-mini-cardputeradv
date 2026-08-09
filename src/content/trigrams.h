// Les huit trigrammes, dans l'ordre du design system ☰☱☲☳☴☵☶☷
// (table TRIGRAMS de ui_kits/daoa-mini/Screens.jsx). Le trigramme d'une
// figure se DÉRIVE de ses traits (HANDOFF-v2) : haut = lignes 4-5-6,
// bas = lignes 1-2-3.
#pragma once

#include <cstdint>

#include "content.h"

namespace daoa {

struct TrigramInfo {
    uint8_t maskIndex;  // 1..8, fichier trigramme-<i>-24x24
    const char* pinyin;
    const char* fr;
    const char* en;
};

inline constexpr TrigramInfo kTrigrams[8] = {
    {1, "Qián", "le Ciel", "Heaven"},
    {2, "Duì", "le Lac", "Lake"},
    {3, "Lí", "le Feu", "Fire"},
    {4, "Zhèn", "le Tonnerre", "Thunder"},
    {5, "Xùn", "le Vent", "Wind"},
    {6, "Kǎn", "l'Eau", "Water"},
    {7, "Gèn", "la Montagne", "Mountain"},
    {8, "Kūn", "la Terre", "Earth"},
};

// bits = 3 traits d'un trigramme, bit 0 = trait bas, 1 = yang.
// Renvoie l'entrée de kTrigrams (vérifié par la figure 6 : bas 010 → Kǎn,
// haut 111 → Qián, comme le gabarit ScreenStructure).
inline const TrigramInfo& trigramFromBits(uint8_t bits) {
    constexpr uint8_t kByBits[8] = {8, 4, 6, 2, 7, 3, 5, 1};  // ☷☳☵☱☶☲☴☰
    return kTrigrams[kByBits[bits & 7] - 1];
}

inline const char* trigramName(const TrigramInfo& t, Lang l) {
    return l == Lang::kFr ? t.fr : t.en;
}

}  // namespace daoa
