// Sons d'interface : sinusoïdes brèves à décroissance exponentielle
// (jamais le tone() carré), volume bas, désactivables dans les réglages.
// Les six traits égrènent une pentatonique montante (la mineur), du bas
// vers le haut ; une ligne mutable double la note à l'octave inférieure.
#pragma once

#include <cstdint>

namespace daoa {

enum class Sound : uint8_t {
    kReveal1 = 0,  // trait 1 (bas) … 6 (haut) : A3 C4 D4 E4 G4 A4
    kReveal2,
    kReveal3,
    kReveal4,
    kReveal5,
    kReveal6,
    kMutation,  // doublage grave discret, joué avec la note du trait
    kComplete,  // résolution douce à l'arrivée du résultat
    kChime,     // fin de la lecture du lieu
    kTick,      // pages, bascules
};

struct SoundHost {
    void (*play)(Sound s);
};

}  // namespace daoa
