#!/usr/bin/env python3
"""Extrait le contenu Yi Jing de Daoa v1 vers des en-têtes C++ générés.

Sources (dépôt v1, jamais modifié) :
  - shared/data/hexagrams.ts        : 64 figures, textes FR/EN (contenu Daoa)
  - shared/logic/hexagramCalculator.ts : table binaire → King Wen (ordre Javary)

Sorties (ne pas éditer à la main) :
  - src/core/kingwen_table.h        : table 64 entrées, bit 0 = ligne basse
  - src/content/hexagrams_data.h    : struct + 64 figures UTF-8

Usage :
    DAOA_V1_PATH=... DAOA_IOS_PATH=... python3 scripts/extract_content.py
    python3 scripts/extract_content.py <chemin_v1> <chemin_daoa_ios>
"""

from __future__ import annotations

import os
import re
import sys
from pathlib import Path

# Sources privées, hors de ce dépôt. Renseigner par variable
# d'environnement, ou passer les chemins en arguments.
#   DAOA_V1_PATH  : le prototype Daoa v1 (mécanique + textes courts)
#   DAOA_IOS_PATH : Daoa 2 iOS (corpus détaillé des 64 figures)
DEFAULT_V1 = os.environ.get("DAOA_V1_PATH", "")
DEFAULT_V2 = os.environ.get("DAOA_IOS_PATH", "")
ROOT = Path(__file__).resolve().parent.parent

SWIFT_STR = r'"((?:[^"\\]|\\.)*)"'
GENERAL_RE = re.compile(
    r"(\d+):\s*\(\s*en:\s*" + SWIFT_STR + r",\s*fr:\s*" + SWIFT_STR + r"\s*\)", re.S
)
CLASSIC_NUM_RE = re.compile(r"(\d+):\s*\(\s*en:\s*ClassicText\(")
CLASSIC_RE = re.compile(
    r"ClassicText\(\s*judgment:\s*" + SWIFT_STR + r",\s*image:\s*" + SWIFT_STR
    + r",\s*lines:\s*\[\s*((?:" + SWIFT_STR + r"\s*,?\s*)+)\]\s*\)",
    re.S,
)
LINE_STR_RE = re.compile(SWIFT_STR, re.S)


def unescape_swift(s: str) -> str:
    return s.replace('\\"', '"').replace("\\\\", "\\")

ENTRY_RE = re.compile(
    r"\{\s*number:\s*(\d+)\s*,"
    r"\s*name:\s*\"((?:[^\"\\]|\\.)*)\"\s*,"
    r"\s*description:\s*\"((?:[^\"\\]|\\.)*)\"\s*,"
    r"\s*nameEn:\s*\"((?:[^\"\\]|\\.)*)\"\s*,"
    r"\s*descriptionEn:\s*\"((?:[^\"\\]|\\.)*)\"\s*\}",
    re.S,
)
TABLE_RE = re.compile(r"'([01]{6})':\s*(\d+)")


def cstr(s: str) -> str:
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def split_name(name: str) -> tuple[str, str, str]:
    """« 乾 Qián - Le Créatif » → (乾, Qián, Le Créatif)."""
    left, sep, title = name.partition(" - ")
    if not sep:
        raise ValueError(f"nom sans titre : {name!r}")
    hanzi, sep, pinyin = left.partition(" ")
    if not sep:
        raise ValueError(f"nom sans pinyin : {name!r}")
    return hanzi.strip(), pinyin.strip(), title.strip()


def source(argv_index: int, default: str, env_name: str, label: str) -> Path:
    value = sys.argv[argv_index] if len(sys.argv) > argv_index else default
    if not value:
        raise SystemExit(
            f"Chemin de {label} inconnu : passer ${env_name} ou l'argument "
            f"n°{argv_index}."
        )
    path = Path(value)
    if not path.is_dir():
        raise SystemExit(f"{label} introuvable : {path}")
    return path


def main() -> None:
    v1 = source(1, DEFAULT_V1, "DAOA_V1_PATH", "Daoa v1")
    hex_src = (v1 / "shared/data/hexagrams.ts").read_text(encoding="utf-8")
    calc_src = (v1 / "shared/logic/hexagramCalculator.ts").read_text(encoding="utf-8")

    entries = {}
    for m in ENTRY_RE.finditer(hex_src):
        number = int(m.group(1))
        hanzi, pinyin, title_fr = split_name(m.group(2))
        _, _, title_en = m.group(4).partition(" - ")
        entries[number] = {
            "hanzi": hanzi,
            "pinyin": pinyin,
            "title_fr": title_fr,
            "title_en": title_en.strip(),
            "desc_fr": m.group(3),
            "desc_en": m.group(5),
        }
    if sorted(entries) != list(range(1, 65)):
        raise SystemExit(f"figures extraites : {len(entries)} (attendu : 64, numéros 1..64)")

    # Table v1 : chaîne binaire écrite de HAUT en BAS ('000001' → seule la
    # ligne basse est yang). Convention interne : bit i = ligne i depuis le
    # BAS, donc int(chaine, 2) donne directement notre index.
    table = {}
    for bits_str, number in TABLE_RE.findall(calc_src):
        table[int(bits_str, 2)] = int(number)
    if sorted(table) != list(range(64)) or sorted(table.values()) != list(range(1, 65)):
        raise SystemExit("table binaire → King Wen non bijective")
    assert table[0b111111] == 1 and table[0b000000] == 2 and table[0b000001] == 24

    lines_bits = {number: bits for bits, number in table.items()}

    gen_note = "// GÉNÉRÉ par scripts/extract_content.py — NE PAS ÉDITER À LA MAIN.\n"

    kingwen = [gen_note]
    kingwen.append("#pragma once\n\n#include <cstdint>\n\nnamespace daoa {\n\n")
    kingwen.append("// Numéro King Wen (1..64) indexé par les 6 lignes réduites yin/yang.\n")
    kingwen.append("// bit i = ligne i depuis le bas, 1 = yang. Ordre Javary/Faure (Daoa v1).\n")
    kingwen.append("inline constexpr uint8_t kBinaryToKingWen[64] = {\n")
    for row in range(8):
        vals = ", ".join(f"{table[row * 8 + c]:2d}" for c in range(8))
        kingwen.append(f"    {vals},\n")
    kingwen.append("};\n\n}  // namespace daoa\n")
    (ROOT / "src/core/kingwen_table.h").write_text("".join(kingwen), encoding="utf-8")

    out = [gen_note]
    out.append("#pragma once\n\n#include <cstdint>\n\nnamespace daoa {\n\n")
    out.append(
        "struct HexagramInfo {\n"
        "    uint8_t number;     // King Wen 1..64\n"
        "    uint8_t linesBits;  // bit i = ligne i depuis le bas, 1 = yang\n"
        "    const char* hanzi;\n"
        "    const char* pinyin;\n"
        "    const char* titleFr;\n"
        "    const char* titleEn;\n"
        "    const char* descFr;\n"
        "    const char* descEn;\n"
        "};\n\n"
    )
    out.append("// Indexée par numéro King Wen − 1. Contenu Daoa original (extrait de v1).\n")
    out.append("inline constexpr HexagramInfo kHexagrams[64] = {\n")
    for n in range(1, 65):
        e = entries[n]
        out.append(
            f"    {{{n}, 0b{lines_bits[n]:06b}, {cstr(e['hanzi'])}, {cstr(e['pinyin'])},\n"
            f"     {cstr(e['title_fr'])}, {cstr(e['title_en'])},\n"
            f"     {cstr(e['desc_fr'])},\n"
            f"     {cstr(e['desc_en'])}}},\n"
        )
    out.append("};\n\n}  // namespace daoa\n")
    (ROOT / "src/content/hexagrams_data.h").write_text("".join(out), encoding="utf-8")

    # ---- Textes détaillés (corpus Daoa 2, daoa-ios) --------------------
    v2 = source(2, DEFAULT_V2, "DAOA_IOS_PATH", "Daoa 2 iOS")
    gen_src = (v2 / "Daoa/Model/HexagramGeneral.swift").read_text(encoding="utf-8")
    cls_src = (v2 / "Daoa/Model/HexagramClassic.swift").read_text(encoding="utf-8")

    generals = {}
    for m in GENERAL_RE.finditer(gen_src):
        generals[int(m.group(1))] = (unescape_swift(m.group(2)), unescape_swift(m.group(3)))
    if sorted(generals) != list(range(1, 65)):
        raise SystemExit(f"portraits généraux : {len(generals)} (attendu 64)")

    cls_numbers = [int(n) for n in CLASSIC_NUM_RE.findall(cls_src)]
    cls_blocks = CLASSIC_RE.findall(cls_src)
    if sorted(cls_numbers) != list(range(1, 65)) or len(cls_blocks) != 128:
        raise SystemExit(
            f"classiques : {len(cls_numbers)} numéros / {len(cls_blocks)} blocs "
            "(attendu 64 / 128, blocs en puis fr)"
        )
    classics = {}
    for i, n in enumerate(cls_numbers):
        entry = {}
        for lang, block in (("en", cls_blocks[2 * i]), ("fr", cls_blocks[2 * i + 1])):
            judgment, image, lines_blob = block[0], block[1], block[2]
            lines = [unescape_swift(x) for x in LINE_STR_RE.findall(lines_blob)]
            if len(lines) != 6:
                raise SystemExit(f"figure {n} ({lang}) : {len(lines)} traits (attendu 6)")
            entry[lang] = (unescape_swift(judgment), unescape_swift(image), lines)
        classics[n] = entry

    det = [gen_note]
    det.append("#pragma once\n\nnamespace daoa {\n\n")
    det.append(
        "// Textes détaillés des 64 figures : portrait général, Jugement, Image\n"
        "// et sens des six traits (bas → haut). Contenu Daoa original, extrait\n"
        "// du corpus Daoa 2 (daoa-ios, rédaction propre d'après Legge 1882 et\n"
        "// de Harlez 1889, domaine public).\n"
        "struct HexagramDetail {\n"
        "    const char* generalEn;\n"
        "    const char* generalFr;\n"
        "    const char* judgmentEn;\n"
        "    const char* judgmentFr;\n"
        "    const char* imageEn;\n"
        "    const char* imageFr;\n"
        "    const char* linesEn[6];\n"
        "    const char* linesFr[6];\n"
        "};\n\n"
    )
    det.append("// Indexée par numéro King Wen − 1.\n")
    det.append("inline constexpr HexagramDetail kHexagramDetails[64] = {\n")
    for n in range(1, 65):
        gen_en, gen_fr = generals[n]
        en_j, en_i, en_l = classics[n]["en"]
        fr_j, fr_i, fr_l = classics[n]["fr"]
        det.append(f"    {{  // {n}\n")
        det.append(f"     {cstr(gen_en)},\n     {cstr(gen_fr)},\n")
        det.append(f"     {cstr(en_j)},\n     {cstr(fr_j)},\n")
        det.append(f"     {cstr(en_i)},\n     {cstr(fr_i)},\n")
        det.append("     {" + ",\n      ".join(cstr(x) for x in en_l) + "},\n")
        det.append("     {" + ",\n      ".join(cstr(x) for x in fr_l) + "}},\n")
    det.append("};\n\n}  // namespace daoa\n")
    (ROOT / "src/content/hexagram_details_data.h").write_text("".join(det), encoding="utf-8")

    print("OK : 64 figures, table King Wen bijective, 64 portraits, 64 classiques.")
    print("  → src/core/kingwen_table.h")
    print("  → src/content/hexagrams_data.h")
    print("  → src/content/hexagram_details_data.h")


if __name__ == "__main__":
    main()
