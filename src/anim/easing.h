// Petites primitives d'animation temporelle (aucune dépendance).
#pragma once

#include <cmath>
#include <cstdint>

namespace daoa::anim {

inline float clamp01(float t) { return t < 0.f ? 0.f : (t > 1.f ? 1.f : t); }

inline float easeOutCubic(float t) {
    t = clamp01(t);
    const float u = 1.f - t;
    return 1.f - u * u * u;
}

inline float easeOutQuint(float t) {
    t = clamp01(t);
    const float u = 1.f - t;
    return 1.f - u * u * u * u * u;
}

inline float easeInOutQuad(float t) {
    t = clamp01(t);
    return t < 0.5f ? 2.f * t * t : 1.f - 2.f * (1.f - t) * (1.f - t);
}

// Respiration sinusoïdale 0..1 (période en ms).
inline float pulse(uint32_t nowMs, uint32_t periodMs) {
    const float phase = static_cast<float>(nowMs % periodMs) / static_cast<float>(periodMs);
    return 0.5f - 0.5f * std::cos(phase * 6.2831853f);
}

// Fenêtre temporelle : progression 0..1 entre startMs et startMs+durationMs.
struct Timeline {
    uint32_t startMs = 0;
    uint32_t durationMs = 1;

    void start(uint32_t now, uint32_t duration) {
        startMs = now;
        durationMs = duration ? duration : 1;
    }
    float progress(uint32_t now) const {
        if (now <= startMs) return 0.f;
        return clamp01(static_cast<float>(now - startMs) / static_cast<float>(durationMs));
    }
    bool done(uint32_t now) const { return progress(now) >= 1.f; }
};

// Interpolation de couleurs RGB565 (sert d'opacité simulée sur fond uni).
inline uint16_t lerp565(uint16_t a, uint16_t b, float t) {
    t = clamp01(t);
    const int ar = (a >> 11) & 0x1F, ag = (a >> 5) & 0x3F, ab = a & 0x1F;
    const int br = (b >> 11) & 0x1F, bg = (b >> 5) & 0x3F, bb = b & 0x1F;
    const int r = ar + static_cast<int>((br - ar) * t + 0.5f);
    const int g = ag + static_cast<int>((bg - ag) * t + 0.5f);
    const int bl = ab + static_cast<int>((bb - ab) * t + 0.5f);
    return static_cast<uint16_t>((r << 11) | (g << 5) | bl);
}

}  // namespace daoa::anim
