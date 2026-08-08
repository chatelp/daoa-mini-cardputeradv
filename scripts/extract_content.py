#!/usr/bin/env python3
"""Extrait le contenu Yi Jing de Daoa v1 vers des en-têtes C++ générés.

Sources (dépôt v1, jamais modifié) :
  - shared/data/hexagrams.ts        : 64 figures, textes FR/EN (contenu Daoa)
  - shared/logic/hexagramCalculator.ts : table binaire → King Wen (ordre Javary)

Sorties (ne pas éditer à la main) :
  - src/core/kingwen_table.h        : table 64 entrées, bit 0 = ligne basse
  - src/content/hexagrams_data.h    : struct + 64 figures UTF-8

Usage : python3 scripts/extract_content.py [chemin_depot_v1]
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

DEFAULT_V1 = "$DAOA_V1_PATH"
ROOT = Path(__file__).resolve().parent.parent

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


def main() -> None:
    v1 = Path(sys.argv[1] if len(sys.argv) > 1 else DEFAULT_V1)
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

    print("OK : 64 figures, table King Wen bijective.")
    print("  → src/core/kingwen_table.h")
    print("  → src/content/hexagrams_data.h")


if __name__ == "__main__":
    main()
