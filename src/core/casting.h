// Logique du tirage ligne à ligne. C++ pur, zéro dépendance Arduino/M5.
// Spécification : docs/03_MECANIQUE_TIRAGE.md.
#pragma once

#include <cstddef>
#include <cstdint>

namespace daoa {

// Source d'aléa injectée : esp_random() sur l'appareil, PRNG en test/sim.
using RandomFn = uint32_t (*)();

// Valeur rituelle d'une ligne (méthode des trois pièces).
enum class LineValue : uint8_t {
    kOldYin = 6,     // vieux yin, mutable → yang
    kYoungYang = 7,  // jeune yang, fixe
    kYoungYin = 8,   // jeune yin, fixe
    kOldYang = 9,    // vieux yang, mutable → yin
};

constexpr bool isYang(LineValue v) {
    return v == LineValue::kYoungYang || v == LineValue::kOldYang;
}

constexpr bool isMoving(LineValue v) {
    return v == LineValue::kOldYin || v == LineValue::kOldYang;
}

// Mutation : 6 → 7, 9 → 8, lignes jeunes inchangées.
constexpr LineValue mutated(LineValue v) {
    if (v == LineValue::kOldYin) return LineValue::kYoungYang;
    if (v == LineValue::kOldYang) return LineValue::kYoungYin;
    return v;
}

// Trois pièces : 3 bits équiprobables, valeur = 6 + somme.
// P(6) = 1/8, P(7) = 3/8, P(8) = 3/8, P(9) = 1/8.
LineValue castLineThreeCoins(RandomFn rand32);

// Numéro King Wen (1..64) des 6 lignes réduites yin/yang.
// lines[0] = ligne basse. Si `afterMutation`, les lignes mutables sont
// d'abord transformées (figure complémentaire).
uint8_t kingWenNumber(const LineValue* lines, bool afterMutation);

// Machine à états du tirage : 6 lignes, de bas en haut, une par appel.
class CastSession {
  public:
    static constexpr size_t kLineCount = 6;

    explicit CastSession(RandomFn rand32) : rand_(rand32) {}

    void reset() { count_ = 0; }

    // Tire la ligne suivante. Renvoie false si le tirage est déjà complet.
    bool castNextLine();

    size_t lineCount() const { return count_; }
    bool isComplete() const { return count_ == kLineCount; }

    // i = 0 → ligne basse. Valide pour i < lineCount().
    LineValue line(size_t i) const { return lines_[i]; }

    bool hasMovingLines() const;

    // Valides uniquement si isComplete().
    uint8_t originalNumber() const { return kingWenNumber(lines_, false); }
    uint8_t complementaryNumber() const { return kingWenNumber(lines_, true); }

  private:
    RandomFn rand_;
    LineValue lines_[kLineCount] = {};
    size_t count_ = 0;
};

}  // namespace daoa
