// Palette et constantes visuelles. Interface sombre : encre de nuit,
// texte papier, un accent cinabre. Décision : retour appareil 2026-08-08
// (fond noir bien plus flatteur sur le petit IPS rétroéclairé).
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

// Fond noir pur (retour appareil 2026-08-08 : se fond avec la lunette de
// l'écran), texte papier, gris pierre pour le secondaire, cinabre éclairci
// pour rester lisible sur fond sombre.
inline constexpr uint16_t kBg = rgb565(0x000000);
inline constexpr uint16_t kFg = rgb565(0xECE4D2);
inline constexpr uint16_t kMuted = rgb565(0x948B7A);
inline constexpr uint16_t kAccent = rgb565(0xC65B42);

// Écran
inline constexpr int kWidth = 240;
inline constexpr int kHeight = 135;

// Taille de fonte minimale lisible sur l'écran physique : efont 12.
// (efont 10 jugée quasi illisible sur l'appareil — retour 2026-08-08.)

// Géométrie des traits (écran de tirage)
inline constexpr int kLineWidth = 116;   // largeur d'un trait plein
inline constexpr int kLineHeight = 8;    // épaisseur
inline constexpr int kLineGap = 24;      // vide central d'un trait brisé
inline constexpr int kLineSpacing = 17;  // pas vertical entre traits

}  // namespace daoa::theme
