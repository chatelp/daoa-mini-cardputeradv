#include "casting.h"

#include "ambient.h"
#include "kingwen_table.h"

namespace daoa {

LineValue castLineThreeCoins(RandomFn rand32) {
    const uint32_t r = rand32();
    const uint32_t coins = ((r >> 0) & 1u) + ((r >> 1) & 1u) + ((r >> 2) & 1u);
    return static_cast<LineValue>(6u + coins);
}

uint8_t kingWenNumber(const LineValue* lines, bool afterMutation) {
    uint8_t bits = 0;
    for (size_t i = 0; i < CastSession::kLineCount; ++i) {
        const LineValue v = afterMutation ? mutated(lines[i]) : lines[i];
        if (isYang(v)) bits |= static_cast<uint8_t>(1u << i);
    }
    return kBinaryToKingWen[bits];
}

bool CastSession::castNextLine() {
    if (isComplete()) return false;
    const CastWeights w{yangBias_, agingProb_};
    lines_[count_++] = castLineThreeCoinsWeighted(rand_, w);
    return true;
}

bool CastSession::hasMovingLines() const {
    for (size_t i = 0; i < count_; ++i) {
        if (isMoving(lines_[i])) return true;
    }
    return false;
}

}  // namespace daoa
