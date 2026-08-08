# Spécification du tirage

Source de vérité de la mécanique. Comportement conforme à Daoa v1
(`shared/logic/hexagramCalculator.ts`, observé le 2026-08-08), **sauf** les
probabilités (décision 2026-08-08 : trois pièces traditionnelles).

## Valeurs de ligne

Chaque ligne vaut une des quatre valeurs rituelles :

| Valeur | Nature | Trait affiché | Mutable |
|---|---|---|---|
| 6 | vieux yin | brisé, marqué | oui → devient yang (7) |
| 7 | jeune yang | plein | non |
| 8 | jeune yin | brisé | non |
| 9 | vieux yang | plein, marqué | oui → devient yin (8) |

## Probabilités (trois pièces)

Chaque ligne = somme de trois pièces (pile = 2, face = 3) :
P(6) = 1/8, P(7) = 3/8, P(8) = 3/8, P(9) = 1/8.
Implémentation : 3 bits indépendants du TRNG, valeur = 6 + bit₁ + bit₂ + bit₃.

## Déroulé

1. Six lignes tirées une à une, **de bas en haut** (la première ligne tirée
   est la ligne du bas). Chaque tirage de ligne est déclenché par
   l'utilisateur (touche ; geste IMU en option future).
2. Aléa : `esp_random()` exclusivement. Aucune entrée utilisateur, aucun
   état de l'app n'influence les valeurs.
3. Figure originale : les 6 lignes réduites à yin/yang (6,8 → yin ;
   7,9 → yang) donnent 6 bits → numéro King Wen via table de correspondance.
4. Figure complémentaire : lignes mutées (6 → 7, 9 → 8, autres inchangées),
   même réduction, même table. Elle est calculée et affichée même si aucune
   ligne n'est mutable (comportement v1 : les deux figures sont alors
   identiques — l'UI peut alors dire « figure stable » plutôt que de
   répéter la même carte ; à valider visuellement).

## Table binaire → King Wen

Convention interne : bit *i* = ligne *i* en partant du **bas** (bit 0 =
première ligne tirée), 1 = yang. La table des 64 entrées est **générée** par
`scripts/extract_content.py` depuis la table de v1 (ordre Javary/Faure) et
vérifiée par tests (64 numéros distincts de 1 à 64 ; 111111 → 1 ;
000000 → 2 ; cas croisés connus).

Attention à la convention v1 : ses chaînes binaires sont écrites de **haut
en bas** (`'000001' → 24` signifie seule la ligne du bas est yang). Le
script fait la conversion vers notre convention bit 0 = bas.

## Tests exigés (natif, `pio test`)

- Distribution des trois pièces (χ² grossier sur large échantillon avec RNG
  déterministe injecté).
- Table : bijection 0..63 → 1..64, cas de référence (1, 2, 11, 12, 63, 64).
- Complémentaire : mutation 6→7 / 9→8, identité sans ligne mutable,
  cas connu avec mutation.
- La machine à états : 6 tirages exactement, pas de 7e, reset propre.
