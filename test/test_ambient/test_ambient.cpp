// Tests de la lecture du lieu : bornes des pondérations, neutralité quand
// désactivée, et effet statistique borné. Spécification : docs/03.
#include <unity.h>

#include "core/ambient.h"
#include "core/casting.h"

using namespace daoa;

void setUp() {}
void tearDown() {}

static uint32_t g_lcg = 12345u;
static uint32_t lcgRand() {
    g_lcg = g_lcg * 1664525u + 1013904223u;
    return g_lcg;
}

// --- bornes -------------------------------------------------------------

void test_poids_bornes_extremes() {
    // Lieu saturé et agité au maximum.
    ApReading dense[24];
    for (int i = 0; i < 24; ++i) {
        dense[i] = {static_cast<int8_t>(i % 2 ? -30 : -90),
                    static_cast<uint8_t>(1 + (i % 13))};
    }
    const CastWeights wMax = weightsFromField(readField(dense, 24));
    TEST_ASSERT_TRUE(wMax.yangBias <= kMaxYangBias + 1e-6f);
    TEST_ASSERT_TRUE(wMax.agingProb <= kMaxAgingProb + 1e-6f);

    // Lieu vide : biais plein yin, aucune agitation.
    const CastWeights wEmpty = weightsFromField(readField(nullptr, 0));
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, -kMaxYangBias, wEmpty.yangBias);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.f, wEmpty.agingProb);
}

void test_densite_monotone() {
    ApReading one[1] = {{-60, 6}};
    ApReading many[10];
    for (int i = 0; i < 10; ++i) many[i] = {-50, 6};
    const AmbientField fOne = readField(one, 1);
    const AmbientField fMany = readField(many, 10);
    TEST_ASSERT_TRUE(fMany.density01 > fOne.density01);
}

// --- neutralité ---------------------------------------------------------

void test_poids_nuls_egale_tradition() {
    // β = γ = 0 : distribution des trois pièces traditionnelle exacte.
    const CastWeights off{0.f, 0.f};
    const int n = 80000;
    int counts[4] = {0};
    g_lcg = 424242u;
    for (int i = 0; i < n; ++i) {
        counts[static_cast<uint8_t>(castLineThreeCoinsWeighted(lcgRand, off)) - 6]++;
    }
    const float expected[4] = {0.125f, 0.375f, 0.375f, 0.125f};
    for (int v = 0; v < 4; ++v) {
        TEST_ASSERT_FLOAT_WITHIN(0.015f, expected[v],
                                 static_cast<float>(counts[v]) / n);
    }
}

// --- effet borné --------------------------------------------------------

void test_biais_plafond_reste_doux() {
    // Aux plafonds : P(yang) monte mais reste ≤ 0,58 ; P(vieux yin) ∈ [6 %, 16 %].
    const CastWeights wMax{kMaxYangBias, kMaxAgingProb};
    const int n = 120000;
    int yang = 0, oldYin = 0, moving = 0;
    g_lcg = 777u;
    for (int i = 0; i < n; ++i) {
        const LineValue v = castLineThreeCoinsWeighted(lcgRand, wMax);
        if (isYang(v)) ++yang;
        if (v == LineValue::kOldYin) ++oldYin;
        if (isMoving(v)) ++moving;
    }
    const float pYang = static_cast<float>(yang) / n;
    const float pOldYin = static_cast<float>(oldYin) / n;
    const float pMoving = static_cast<float>(moving) / n;
    TEST_ASSERT_TRUE(pYang > 0.52f && pYang < 0.58f);   // 0,55 attendu
    TEST_ASSERT_TRUE(pOldYin > 0.10f && pOldYin < 0.16f);  // 0,45 × 0,29 ≈ 0,13
    // Mutabilité : 0,25 + γ = 0,29 au plafond.
    TEST_ASSERT_TRUE(pMoving > 0.26f && pMoving < 0.32f);
}

void test_session_pondérée_reste_valide() {
    CastSession s(lcgRand);
    s.setWeights(kMaxYangBias, kMaxAgingProb);
    while (!s.isComplete()) s.castNextLine();
    const uint8_t original = s.originalNumber();
    const uint8_t complementary = s.complementaryNumber();
    TEST_ASSERT_TRUE(original >= 1 && original <= 64);
    TEST_ASSERT_TRUE(complementary >= 1 && complementary <= 64);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_poids_bornes_extremes);
    RUN_TEST(test_densite_monotone);
    RUN_TEST(test_poids_nuls_egale_tradition);
    RUN_TEST(test_biais_plafond_reste_doux);
    RUN_TEST(test_session_pondérée_reste_valide);
    return UNITY_END();
}
