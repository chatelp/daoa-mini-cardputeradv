// Synthèse des sons d'interface sur le codec du Cardputer (ES8311, HP 1 W)
// via M5Unified : petites sinusoïdes PCM avec enveloppe exponentielle,
// générées une fois au premier son. Volume volontairement bas.
#ifndef DAOA_SIM

#include <M5Cardputer.h>

#include <cmath>

#include "ui_sound.h"

namespace daoa {

namespace {

constexpr uint32_t kRate = 22050;
constexpr float kVolume = 0.55f;  // discret ; le volume maître reste celui de M5

// Une note : fréquence, durée, retard de départ (mélange dans le buffer).
struct Partial {
    float freq;
    float dur;
    float delay;
    float gain;
};

int16_t* synth(const Partial* parts, int nParts, float totalDur, size_t& outLen) {
    outLen = static_cast<size_t>(totalDur * kRate);
    auto* buf = new int16_t[outLen];
    for (size_t i = 0; i < outLen; ++i) {
        const float t = static_cast<float>(i) / kRate;
        float v = 0.f;
        for (int p = 0; p < nParts; ++p) {
            const float lt = t - parts[p].delay;
            if (lt < 0.f || lt > parts[p].dur) continue;
            const float env = std::exp(-lt * 22.f) * (1.f - std::exp(-lt * 900.f));
            v += parts[p].gain * env * std::sin(6.2831853f * parts[p].freq * lt);
        }
        v *= kVolume;
        if (v > 1.f) v = 1.f;
        if (v < -1.f) v = -1.f;
        buf[i] = static_cast<int16_t>(v * 32767.f);
    }
    return buf;
}

struct Clip {
    const int16_t* data = nullptr;
    size_t len = 0;
};

// Pentatonique la mineur, DEUX OCTAVES AU-DESSUS de l'écriture naturelle :
// A5 C6 D6 E6 G6 A6. Le haut-parleur de 1 W du Cardputer ne restitue
// pratiquement rien sous ~400 Hz — une gamme en A3–A4 est inaudible sur
// l'appareil (constaté le 2026-08-09). Les rapports musicaux sont intacts,
// seul le registre change : tout tombe désormais dans la bande efficace.
constexpr float kScale[6] = {880.f, 1046.5f, 1174.66f, 1318.51f, 1568.f, 1760.f};

Clip makeClip(Sound s) {
    Clip c;
    switch (s) {
        case Sound::kReveal1:
        case Sound::kReveal2:
        case Sound::kReveal3:
        case Sound::kReveal4:
        case Sound::kReveal5:
        case Sound::kReveal6: {
            const int i = static_cast<int>(s);
            const Partial p[] = {{kScale[i], 0.22f, 0.f, 1.f}};
            c.data = synth(p, 1, 0.24f, c.len);
            break;
        }
        case Sound::kMutation: {
            // Une quinte au-dessus de la tonique, un souffle après le trait :
            // la mutation s'entend sans descendre hors de la bande utile.
            const Partial p[] = {{1318.51f, 0.26f, 0.06f, 0.7f}};
            c.data = synth(p, 1, 0.34f, c.len);
            break;
        }
        case Sound::kComplete: {
            // Quinte descendante : E6 puis A5.
            const Partial p[] = {{1318.51f, 0.22f, 0.f, 0.9f}, {880.f, 0.36f, 0.14f, 1.f}};
            c.data = synth(p, 2, 0.52f, c.len);
            break;
        }
        case Sound::kChime: {
            // La lecture du lieu se pose : E6 doux, une octave par-dessus.
            const Partial p[] = {{1318.51f, 0.4f, 0.f, 0.5f}, {2637.f, 0.28f, 0.02f, 0.18f}};
            c.data = synth(p, 2, 0.42f, c.len);
            break;
        }
        case Sound::kTick: {
            // Au-dessus de la gamme, pour ne pas s'y confondre.
            const Partial p[] = {{2093.f, 0.03f, 0.f, 0.35f}};
            c.data = synth(p, 1, 0.035f, c.len);
            break;
        }
    }
    return c;
}

void playDevice(Sound s) {
    static Clip clips[10];
    const int idx = static_cast<int>(s);
    if (clips[idx].data == nullptr) clips[idx] = makeClip(s);
    if (clips[idx].data != nullptr) {
        M5Cardputer.Speaker.playRaw(clips[idx].data, clips[idx].len, kRate, false, 1, 0);
    }
}

void setVolumeDevice(uint8_t volume) { M5Cardputer.Speaker.setVolume(volume); }

}  // namespace

extern const SoundHost kDeviceSoundHost;
const SoundHost kDeviceSoundHost{playDevice, setVolumeDevice};

}  // namespace daoa

#endif  // !DAOA_SIM
