// GÉNÉRÉ par scripts/extract_content.py — NE PAS ÉDITER À LA MAIN.
#pragma once

#include <cstdint>

namespace daoa {

// Numéro King Wen (1..64) indexé par les 6 lignes réduites yin/yang.
// bit i = ligne i depuis le bas, 1 = yang. Ordre Javary/Faure (Daoa v1).
inline constexpr uint8_t kBinaryToKingWen[64] = {
     2, 24,  7, 19, 15, 36, 46, 11,
    16, 51, 40, 54, 62, 55, 32, 34,
     8,  3, 29, 60, 39, 63, 48,  5,
    45, 17, 47, 58, 31, 49, 28, 43,
    23, 27,  4, 41, 52, 22, 18, 26,
    35, 21, 64, 38, 56, 30, 50, 14,
    20, 42, 59, 61, 53, 37, 57,  9,
    12, 25,  6, 10, 33, 13, 44,  1,
};

}  // namespace daoa
