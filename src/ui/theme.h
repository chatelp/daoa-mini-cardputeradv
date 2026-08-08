// Palette et constantes visuelles. Encre sur papier, un accent discret.
// Standard : docs/04_UX_RENDU.md — calme, précis, premium, typographique.
#pragma once

#include <cstdint>

namespace daoa::theme {

constexpr uint16_t rgb565(uint32_t rgb888) {
    const uint32_t r = (rgb888 >> 16) & 0xFF;
    const uint32_t g = (rgb888 >> 8) & 0xFF;
    const uint32_t b = rgb888 & 0xFF;
    return static_cast<uint16_t>(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

// Papier chaud, encre profonde, pierre pour le secondaire,
// cinabre assourdi pour les marques de mutation (clin d'œil au sceau).
inline constexpr uint16_t kPaper = rgb565(0xF2ECDF);
inline constexpr uint16_t kInk = rgb565(0x26221B);
inline constexpr uint16_t kStone = rgb565(0x8D8474);
inline constexpr uint16_t kAccent = rgb565(0x9E4A38);

// Écran
inline constexpr int kWidth = 240;
inline constexpr int kHeight = 135;

// Géométrie des traits (écran de tirage)
inline constexpr int kLineWidth = 116;   // largeur d'un trait plein
inline constexpr int kLineHeight = 8;    // épaisseur
inline constexpr int kLineGap = 24;      // vide central d'un trait brisé
inline constexpr int kLineSpacing = 17;  // pas vertical entre traits

}  // namespace daoa::theme
