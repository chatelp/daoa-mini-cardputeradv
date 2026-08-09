// Lecture du lieu : dérive du champ radio ambiant deux lectures et des
// pondérations bornées pour les trois pièces. C++ pur, testé en natif.
// Spécification : docs/03_MECANIQUE_TIRAGE.md, décision du 2026-08-09.
#pragma once

#include <cstddef>
#include <cstdint>

#include "casting.h"

namespace daoa {

// Un réseau perçu : force du signal (dBm, typiquement −90…−30) et canal.
struct ApReading {
    int8_t rssi;
    uint8_t channel;
};

// La lecture du lieu, réduite à deux grandeurs sur [0, 1].
struct AmbientField {
    uint8_t networkCount = 0;
    float density01 = 0.f;    // combien le lieu est habité (nombre + énergie)
    float agitation01 = 0.f;  // combien il est agité (canaux + variance)
};

// Pondérations bornées appliquées au tirage. Construction équivalente à la
// méthode des trois pièces, décomposée en deux Bernoulli indépendantes :
// nature de la ligne (yang avec P = 0,5 + β) puis mutabilité (vieille avec
// P = 0,25 + γ). À β = γ = 0 c'est exactement la distribution
// traditionnelle (1/8, 3/8, 3/8, 1/8). Biaiser la pièce elle-même serait
// sans effet : la parité de trois pièces est insensible au premier ordre.
struct CastWeights {
    float yangBias = 0.f;   // β ∈ [−kMaxYangBias, +kMaxYangBias]
    float agingProb = 0.f;  // γ ∈ [0, kMaxAgingProb]
};

inline constexpr float kMaxYangBias = 0.05f;
inline constexpr float kMaxAgingProb = 0.04f;

// Réduit un scan en lecture du lieu. n = 0 → champ neutre (0.5, 0).
AmbientField readField(const ApReading* aps, size_t n);

// Mappe la lecture sur les pondérations, bornes garanties.
CastWeights weightsFromField(const AmbientField& f);

// Tirage d'une ligne pondérée (deux champs de 10 bits d'un même mot du
// TRNG ; 512/1024 et 256/1024 exacts quand β = γ = 0).
LineValue castLineThreeCoinsWeighted(RandomFn rand32, const CastWeights& w);

}  // namespace daoa
