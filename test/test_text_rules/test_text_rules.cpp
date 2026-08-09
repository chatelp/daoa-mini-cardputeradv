// Tests des règles de texte du design system (HANDOFF-v2, tâches 1 et 2) :
// clampGloss (une branche par cas) et la contrainte dure des 38 colonnes du
// pied de page, pour chaque écran et chaque langue.
#include <string>
#include <unity.h>
#include <vector>

#include "ui/daoa_key_labels.h"
#include "ui/screen_footers.h"
#include "ui/text_rules.h"

using namespace daoa;

void setUp() {}
void tearDown() {}

// --- clampGloss ----------------------------------------------------------

void test_glose_deux_lignes_inchangee() {
    const std::vector<std::string> in = {"Le Litige. Conflit, discorde,",
                                         "nécessité de modération."};
    const auto out = clampGloss(in);
    TEST_ASSERT_EQUAL_size_t(2, out.size());
    TEST_ASSERT_EQUAL_STRING(in[0].c_str(), out[0].c_str());
    TEST_ASSERT_EQUAL_STRING(in[1].c_str(), out[1].c_str());
}

void test_glose_coupure_normale() {
    // 3 lignes : la 2e est coupée au dernier mot entier + « … », la 3e disparaît.
    const std::vector<std::string> in = {
        "Le Ciel. Force créatrice, dynamisme,",
        "initiative. Le principe yang à son",
        "maximum."};
    const auto out = clampGloss(in);
    TEST_ASSERT_EQUAL_size_t(2, out.size());
    TEST_ASSERT_EQUAL_STRING("Le Ciel. Force créatrice, dynamisme,", out[0].c_str());
    // La 2e ligne tient déjà en 37 colonnes : ellipse simplement collée.
    TEST_ASSERT_EQUAL_STRING("initiative. Le principe yang à son…", out[1].c_str());
}

void test_glose_coupure_au_mot() {
    // 2e ligne de 38 colonnes pleines : coupe au dernier mot entier avant 37.
    const std::vector<std::string> in = {
        "première ligne",
        "abcdefghij klmnopqrst uvwxyzabcde fghij",  // 40 colonnes
        "troisième"};
    const auto out = clampGloss(in);
    TEST_ASSERT_EQUAL_size_t(2, out.size());
    TEST_ASSERT_EQUAL_STRING("abcdefghij klmnopqrst uvwxyzabcde…", out[1].c_str());
}

void test_glose_seuil_queue_courte() {
    // La 2e coupée garderait < 12 caractères : elle disparaît, la 1re est coupée.
    const std::vector<std::string> in = {
        "Une première ligne assez longue pour",
        "un mot",
        "et une troisième ligne"};
    const auto out = clampGloss(in);
    TEST_ASSERT_EQUAL_size_t(1, out.size());
    TEST_ASSERT_EQUAL_STRING("Une première ligne assez longue pour…", out[0].c_str());
}

void test_glose_ponctuation_faible() {
    // Une virgule reste en fin de coupe : elle est retirée avant l'ellipse.
    const std::vector<std::string> in = {
        "première",
        "mot mot mot mot mot mot mot mot ab, cdefg",  // coupe après « ab, »
        "suite"};
    const auto out = clampGloss(in);
    TEST_ASSERT_EQUAL_size_t(2, out.size());
    TEST_ASSERT_EQUAL_STRING("mot mot mot mot mot mot mot mot ab…", out[1].c_str());
}

void test_glose_accents_utf8() {
    // 37 « é » = 74 octets mais 37 colonnes : tient, ellipse collée sans coupe.
    const std::string line(37 * 2, ' ');
    std::string es;
    for (int i = 0; i < 37; ++i) es += "é";
    const std::vector<std::string> in = {"première", es, "troisième"};
    const auto out = clampGloss(in);
    TEST_ASSERT_EQUAL_size_t(2, out.size());
    TEST_ASSERT_EQUAL_STRING((es + "…").c_str(), out[1].c_str());
    TEST_ASSERT_EQUAL_INT(38, displayCols(out[1]));
}

// --- contrainte des 38 colonnes du pied ----------------------------------

namespace {
int footerCols(const ui::FooterSpec& spec, Lang lang) {
    int cols = 0;
    if (spec.legendId != nullptr) {
        cols += displayCols(keyLabel(spec.legendId, lang));
        cols += 2;  // respiration minimale légende ↔ raccourcis
    }
    for (int i = 0; i < spec.actionCount; ++i) {
        cols += displayCols(keyLabel(spec.actions[i].key, lang));
        if (spec.actions[i].label != nullptr) {
            cols += 1 + displayCols(keyLabel(spec.actions[i].label, lang));
        }
        if (i + 1 < spec.actionCount) cols += 3;  // « · »
    }
    return cols;
}
}  // namespace

void test_pieds_38_colonnes_toutes_langues() {
    for (const auto& spec : ui::kFooterSpecs) {
        for (Lang lang : {Lang::kFr, Lang::kEn}) {
            const int cols = footerCols(spec, lang);
            char msg[96];
            snprintf(msg, sizeof(msg), "%s (%s) : %d cols", spec.name,
                     lang == Lang::kFr ? "fr" : "en", cols);
            TEST_ASSERT_LESS_OR_EQUAL_INT_MESSAGE(38, cols, msg);
        }
    }
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_glose_deux_lignes_inchangee);
    RUN_TEST(test_glose_coupure_normale);
    RUN_TEST(test_glose_coupure_au_mot);
    RUN_TEST(test_glose_seuil_queue_courte);
    RUN_TEST(test_glose_ponctuation_faible);
    RUN_TEST(test_glose_accents_utf8);
    RUN_TEST(test_pieds_38_colonnes_toutes_langues);
    return UNITY_END();
}
