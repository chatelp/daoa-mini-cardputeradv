// Tests natifs de la logique de tirage. Spécification : docs/03_MECANIQUE_TIRAGE.md.
#include <unity.h>

#include "core/casting.h"
#include "core/kingwen_table.h"

using namespace daoa;

void setUp() {}
void tearDown() {}

// --- aléa contrôlé -------------------------------------------------------

static uint32_t g_fixed;
static uint32_t fixedRand() { return g_fixed; }

// LCG déterministe pour les tests statistiques.
static uint32_t g_lcg = 12345u;
static uint32_t lcgRand() {
    g_lcg = g_lcg * 1664525u + 1013904223u;
    return g_lcg >> 8;
}

// --- table King Wen ------------------------------------------------------

void test_table_est_bijective() {
    int seen[65] = {0};
    for (int i = 0; i < 64; ++i) {
        TEST_ASSERT_GREATER_OR_EQUAL_UINT8(1, kBinaryToKingWen[i]);
        TEST_ASSERT_LESS_OR_EQUAL_UINT8(64, kBinaryToKingWen[i]);
        ++seen[kBinaryToKingWen[i]];
    }
    for (int n = 1; n <= 64; ++n) TEST_ASSERT_EQUAL_INT(1, seen[n]);
}

void test_table_references_connues() {
    TEST_ASSERT_EQUAL_UINT8(1, kBinaryToKingWen[0b111111]);   // tout yang → Qián
    TEST_ASSERT_EQUAL_UINT8(2, kBinaryToKingWen[0b000000]);   // tout yin → Kūn
    TEST_ASSERT_EQUAL_UINT8(24, kBinaryToKingWen[0b000001]);  // yang en bas → Fù
    TEST_ASSERT_EQUAL_UINT8(11, kBinaryToKingWen[0b000111]);  // 3 yang en bas → Tài
    TEST_ASSERT_EQUAL_UINT8(12, kBinaryToKingWen[0b111000]);  // 3 yang en haut → Pǐ
    TEST_ASSERT_EQUAL_UINT8(63, kBinaryToKingWen[0b010101]);  // Jì Jì
    TEST_ASSERT_EQUAL_UINT8(64, kBinaryToKingWen[0b101010]);  // Wèi Jì
}

// --- trois pièces --------------------------------------------------------

void test_trois_pieces_valeurs() {
    g_fixed = 0b000;
    TEST_ASSERT_EQUAL_UINT8(6, static_cast<uint8_t>(castLineThreeCoins(fixedRand)));
    g_fixed = 0b111;
    TEST_ASSERT_EQUAL_UINT8(9, static_cast<uint8_t>(castLineThreeCoins(fixedRand)));
    g_fixed = 0b001;
    TEST_ASSERT_EQUAL_UINT8(7, static_cast<uint8_t>(castLineThreeCoins(fixedRand)));
    g_fixed = 0b011;
    TEST_ASSERT_EQUAL_UINT8(8, static_cast<uint8_t>(castLineThreeCoins(fixedRand)));
}

void test_trois_pieces_distribution() {
    // P(6)=P(9)=1/8, P(7)=P(8)=3/8, tolérance ±1,5 point sur 80 000 tirages.
    const int n = 80000;
    int counts[4] = {0};
    g_lcg = 987654321u;
    for (int i = 0; i < n; ++i) {
        counts[static_cast<uint8_t>(castLineThreeCoins(lcgRand)) - 6]++;
    }
    const float expected[4] = {0.125f, 0.375f, 0.375f, 0.125f};
    for (int v = 0; v < 4; ++v) {
        const float freq = static_cast<float>(counts[v]) / n;
        TEST_ASSERT_FLOAT_WITHIN(0.015f, expected[v], freq);
    }
}

// --- mutation ------------------------------------------------------------

void test_mutation() {
    TEST_ASSERT_EQUAL_UINT8(7, static_cast<uint8_t>(mutated(LineValue::kOldYin)));
    TEST_ASSERT_EQUAL_UINT8(8, static_cast<uint8_t>(mutated(LineValue::kOldYang)));
    TEST_ASSERT_EQUAL_UINT8(7, static_cast<uint8_t>(mutated(LineValue::kYoungYang)));
    TEST_ASSERT_EQUAL_UINT8(8, static_cast<uint8_t>(mutated(LineValue::kYoungYin)));
    TEST_ASSERT_TRUE(isMoving(LineValue::kOldYin));
    TEST_ASSERT_TRUE(isMoving(LineValue::kOldYang));
    TEST_ASSERT_FALSE(isMoving(LineValue::kYoungYang));
    TEST_ASSERT_FALSE(isMoving(LineValue::kYoungYin));
}

// --- session -------------------------------------------------------------

void test_session_tout_vieux_yin() {
    g_fixed = 0b000;  // toutes les lignes = 6 (vieux yin)
    CastSession s(fixedRand);
    for (int i = 0; i < 6; ++i) {
        TEST_ASSERT_FALSE(s.isComplete());
        TEST_ASSERT_TRUE(s.castNextLine());
    }
    TEST_ASSERT_TRUE(s.isComplete());
    TEST_ASSERT_FALSE(s.castNextLine());  // pas de 7e ligne
    TEST_ASSERT_TRUE(s.hasMovingLines());
    TEST_ASSERT_EQUAL_UINT8(2, s.originalNumber());       // tout yin → Kūn
    TEST_ASSERT_EQUAL_UINT8(1, s.complementaryNumber());  // muté tout yang → Qián
}

void test_session_stable() {
    g_fixed = 0b001;  // toutes les lignes = 7 (jeune yang, fixe)
    CastSession s(fixedRand);
    while (!s.isComplete()) s.castNextLine();
    TEST_ASSERT_FALSE(s.hasMovingLines());
    TEST_ASSERT_EQUAL_UINT8(1, s.originalNumber());
    TEST_ASSERT_EQUAL_UINT8(1, s.complementaryNumber());  // identique si stable
}

void test_session_reset() {
    g_fixed = 0b001;
    CastSession s(fixedRand);
    s.castNextLine();
    s.castNextLine();
    TEST_ASSERT_EQUAL_size_t(2, s.lineCount());
    s.reset();
    TEST_ASSERT_EQUAL_size_t(0, s.lineCount());
    TEST_ASSERT_FALSE(s.isComplete());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_table_est_bijective);
    RUN_TEST(test_table_references_connues);
    RUN_TEST(test_trois_pieces_valeurs);
    RUN_TEST(test_trois_pieces_distribution);
    RUN_TEST(test_mutation);
    RUN_TEST(test_session_tout_vieux_yin);
    RUN_TEST(test_session_stable);
    RUN_TEST(test_session_reset);
    return UNITY_END();
}
