// Règles de texte du design system (HANDOFF-v2, tâche 1).
// C++ pur, sans dépendance graphique : testé en natif par pio test.
#pragma once

#include <string>
#include <vector>

namespace daoa {

// Ellipse : U+2026, qui occupe bien 1 colonne (6 px, demi-chasse) dans la
// fonte embarquée efontJA_12 — mesuré au simulateur le 2026-08-09. C'est la
// variante nominale de la règle ; le repli « ... » à 3 colonnes n'est pas
// nécessaire.
inline constexpr const char* kEllipsis = "…";
inline constexpr int kEllipsisCols = 1;

// Nombre de colonnes affichées (codepoints UTF-8, tous demi-chasse en
// latin — les gloses ne contiennent pas de CJK).
int displayCols(const std::string& s);

// Règle officielle de troncature de la glose — port à l'identique de
// clampGloss() (components/text/FigureTitle.jsx) :
//   1. ≤ 2 lignes : inchangé.
//   2. Sinon : garder la 1re ; couper la 2e au dernier mot entier laissant
//      la place à l'ellipse ; retirer une ponctuation faible finale
//      (, ; : — -) ; coller « … » sans espace.
//   3. Si la 2e ainsi coupée garde < minTail caractères, la supprimer et
//      couper la 1re de la même façon.
//   4. Jamais de césure, jamais de compression, jamais de 3e ligne.
std::vector<std::string> clampGloss(const std::vector<std::string>& gloss,
                                    int cols = 38, int minTail = 12);

}  // namespace daoa
