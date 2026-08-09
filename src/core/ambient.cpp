#include "ambient.h"

namespace daoa {

namespace {

constexpr float clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }

// Normalise un RSSI en énergie [0, 1] : −90 dBm → 0, −40 dBm → 1.
constexpr float rssiEnergy(int8_t rssi) {
    return clamp01((static_cast<float>(rssi) + 90.f) / 50.f);
}

}  // namespace

AmbientField readField(const ApReading* aps, size_t n) {
    AmbientField f;
    if (aps == nullptr || n == 0) {
        // Lieu silencieux : densité médiane basse, aucune agitation.
        f.density01 = 0.f;
        f.agitation01 = 0.f;
        return f;
    }
    f.networkCount = static_cast<uint8_t>(n > 24 ? 24 : n);

    // Densité : moitié nombre de réseaux (saturé à 10), moitié énergie moyenne.
    float energySum = 0.f;
    for (size_t i = 0; i < n; ++i) energySum += rssiEnergy(aps[i].rssi);
    const float countPart = clamp01(static_cast<float>(n) / 10.f);
    const float energyPart = energySum / static_cast<float>(n);
    f.density01 = clamp01(0.5f * countPart + 0.5f * energyPart);

    // Agitation : moitié étalement des canaux, moitié variance des énergies.
    bool seen[15] = {};
    int channels = 0;
    for (size_t i = 0; i < n; ++i) {
        const uint8_t c = aps[i].channel <= 14 ? aps[i].channel : 14;
        if (!seen[c]) {
            seen[c] = true;
            ++channels;
        }
    }
    const float spreadPart = n > 1 ? clamp01(static_cast<float>(channels - 1) / 6.f) : 0.f;
    float variance = 0.f;
    for (size_t i = 0; i < n; ++i) {
        const float d = rssiEnergy(aps[i].rssi) - energyPart;
        variance += d * d;
    }
    variance /= static_cast<float>(n);
    const float variancePart = clamp01(variance / 0.09f);  // σ = 0,3 → 1
    f.agitation01 = clamp01(0.5f * spreadPart + 0.5f * variancePart);
    return f;
}

CastWeights weightsFromField(const AmbientField& f) {
    CastWeights w;
    // Densité 0,5 = neutre ; saturé → yang, silencieux → yin.
    w.yangBias = (clamp01(f.density01) - 0.5f) * 2.f * kMaxYangBias;
    w.agingProb = clamp01(f.agitation01) * kMaxAgingProb;
    return w;
}

LineValue castLineThreeCoinsWeighted(RandomFn rand32, const CastWeights& w) {
    const uint32_t r = rand32();
    // Nature de la ligne : yang avec P = 0,5 + β.
    const uint32_t yangT =
        static_cast<uint32_t>((0.5f + w.yangBias) * 1024.f + 0.5f);
    const bool yang = (r & 0x3FF) < yangT;
    // Mutabilité : vieille avec P = 0,25 + γ.
    const uint32_t oldT =
        static_cast<uint32_t>((0.25f + w.agingProb) * 1024.f + 0.5f);
    const bool old = ((r >> 10) & 0x3FF) < oldT;
    if (yang) return old ? LineValue::kOldYang : LineValue::kYoungYang;
    return old ? LineValue::kOldYin : LineValue::kYoungYin;
}

}  // namespace daoa
