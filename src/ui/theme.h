// Alias de compatibilité vers les tokens du design system (daoa_tokens.h).
// Toute nouvelle cote passe par daoa::tokens ; ce fichier n'existe que pour
// les points d'entrée historiques (simulateur, captures).
#pragma once

#include "daoa_tokens.h"

namespace daoa::theme {

using daoa::tokens::rgb565;

inline constexpr uint16_t kBg = tokens::MINI_BG;
inline constexpr uint16_t kFg = tokens::MINI_TEXT;
inline constexpr uint16_t kMuted = tokens::MINI_TEXT_META;
inline constexpr uint16_t kAccent = tokens::MINI_MUTATION;

inline constexpr int kWidth = tokens::MINI_W;
inline constexpr int kHeight = tokens::MINI_H;

}  // namespace daoa::theme
