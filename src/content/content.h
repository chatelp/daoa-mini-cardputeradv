// Accès typé au contenu généré (langue, figure).
#pragma once

#include "hexagrams_data.h"

namespace daoa {

enum class Lang : uint8_t { kFr, kEn };

inline const HexagramInfo& hexagram(uint8_t kingWen) {
    return kHexagrams[kingWen - 1];
}

inline const char* title(const HexagramInfo& h, Lang l) {
    return l == Lang::kFr ? h.titleFr : h.titleEn;
}

inline const char* description(const HexagramInfo& h, Lang l) {
    return l == Lang::kFr ? h.descFr : h.descEn;
}

}  // namespace daoa
