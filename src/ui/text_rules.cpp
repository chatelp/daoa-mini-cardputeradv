#include "text_rules.h"

namespace daoa {

namespace {

int cpBytes(unsigned char b) {
    if (b < 0x80) return 1;
    if ((b >> 5) == 0b110) return 2;
    if ((b >> 4) == 0b1110) return 3;
    return 4;
}

// Préfixe de `cols` colonnes affichées (coupe aux frontières UTF-8).
std::string firstCols(const std::string& s, int cols) {
    size_t i = 0;
    int n = 0;
    while (i < s.size() && n < cols) {
        i += static_cast<size_t>(cpBytes(static_cast<unsigned char>(s[i])));
        ++n;
    }
    return s.substr(0, i);
}

// Retire UNE ponctuation faible finale : , ; : - ou — (U+2014).
void stripWeakPunctuation(std::string& s) {
    if (s.empty()) return;
    const char c = s.back();
    if (c == ',' || c == ';' || c == ':' || c == '-') {
        s.pop_back();
    } else if (s.size() >= 3 && s.compare(s.size() - 3, 3, "\xE2\x80\x94") == 0) {
        s.erase(s.size() - 3);
    }
}

std::string cut(const std::string& s, int cols) {
    if (displayCols(s) <= cols - kEllipsisCols) return s + kEllipsis;
    const std::string t = firstCols(s, cols - kEllipsisCols);
    const size_t sp = t.rfind(' ');
    std::string base = (sp != std::string::npos && sp > 0) ? t.substr(0, sp) : t;
    stripWeakPunctuation(base);
    return base + kEllipsis;
}

}  // namespace

int displayCols(const std::string& s) {
    int n = 0;
    for (size_t i = 0; i < s.size();
         i += static_cast<size_t>(cpBytes(static_cast<unsigned char>(s[i])))) {
        ++n;
    }
    return n;
}

std::vector<std::string> clampGloss(const std::vector<std::string>& gloss, int cols,
                                    int minTail) {
    if (gloss.size() <= 2) return gloss;
    const std::string second = cut(gloss[1], cols);
    if (displayCols(second) - kEllipsisCols >= minTail) return {gloss[0], second};
    return {cut(gloss[0], cols)};
}

}  // namespace daoa
