#!/usr/bin/env python3
"""Convertit les ressources graphiques en en-têtes C++ embarqués.

Deux sources :
  1. Le lettermark « daoa » de daoa-ios (WordmarkSplash/dark), recadré et
     rééchantillonné à la largeur du design system (104 px sur l'accueil).
  2. Les assets bitmap du design system (.claude/skills/daoa-mini-design/
     assets/bitmap) : masques monochromes (canal alpha = couverture) et fond
     promo précomposé sur noir.

Sorties (ne pas éditer à la main) :
  - src/ui/assets/wordmark_data.h   masque 8 bits + couleur de marque
  - src/ui/assets/ds_assets.h       masques du design system + fond promo 565

Usage : DAOA_IOS_PATH=... python3 scripts/convert_assets.py
"""

import os
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent

# Source privée, hors de ce dépôt : $DAOA_IOS_PATH doit pointer sur le
# dépôt Daoa 2 iOS, d'où vient le lettermark.
DAOA_IOS = Path(os.environ.get("DAOA_IOS_PATH", ""))
WORDMARK_SRC = (
    DAOA_IOS / "Daoa/Resources/Assets.xcassets/WordmarkSplash.imageset/dark.png"
)
BITMAP_DIR = ROOT / ".claude/skills/daoa-mini-design/assets/bitmap"
OUT_DIR = ROOT / "src/ui/assets"

WORDMARK_WIDTH = 104  # gabarit accueil du design system

GEN_NOTE = "// GÉNÉRÉ par scripts/convert_assets.py — NE PAS ÉDITER À LA MAIN.\n"

# Masques à embarquer : (fichier, nom C++)
MASKS = [
    ("mutation-yang-16.png", "MutationYang16"),
    ("mutation-yang-12.png", "MutationYang12"),
    ("mutation-yang-8.png", "MutationYang8"),
    ("mutation-yin-16.png", "MutationYin16"),
    ("mutation-yin-12.png", "MutationYin12"),
    ("mutation-yin-8.png", "MutationYin8"),
    ("accueil-ornement-140x8.png", "OrnementAccueil"),
    ("sceau-36x46.png", "Sceau"),
    ("trigramme-1-24x24.png", "Trigramme1"),
    ("trigramme-2-24x24.png", "Trigramme2"),
    ("trigramme-3-24x24.png", "Trigramme3"),
    ("trigramme-4-24x24.png", "Trigramme4"),
    ("trigramme-5-24x24.png", "Trigramme5"),
    ("trigramme-6-24x24.png", "Trigramme6"),
    ("trigramme-7-24x24.png", "Trigramme7"),
    ("trigramme-8-24x24.png", "Trigramme8"),
]


def emit_bytes(data: bytes, per_line: int = 24) -> str:
    out = []
    for i in range(0, len(data), per_line):
        out.append("    " + ",".join(str(b) for b in data[i : i + per_line]) + ",\n")
    return "".join(out)


def alpha_mask(im: Image.Image) -> bytes:
    return im.convert("RGBA").getchannel("A").tobytes()


def write_wordmark() -> None:
    im = Image.open(WORDMARK_SRC).convert("RGBA")
    im = im.crop(im.getchannel("A").getbbox())

    px = im.load()
    rs = gs = bs = n = 0
    for y in range(0, im.height, 7):
        for x in range(0, im.width, 7):
            r, g, b, a = px[x, y]
            if a > 250:
                rs, gs, bs, n = rs + r, gs + g, bs + b, n + 1
    color = (rs // n, gs // n, bs // n)

    h = round(im.height * WORDMARK_WIDTH / im.width)
    small = im.resize((WORDMARK_WIDTH, h), Image.LANCZOS).getchannel("A")
    data = small.tobytes()

    out = [GEN_NOTE]
    out.append("// Lettermark « daoa » (Daoa 2, WordmarkSplash/dark) : masque de\n")
    out.append("// couverture 8 bits, teinté MINI_BRAND à l'exécution.\n")
    out.append("#pragma once\n\n#include <cstdint>\n\nnamespace daoa {\n\n")
    out.append(f"inline constexpr int kWordmarkWidth = {small.width};\n")
    out.append(f"inline constexpr int kWordmarkHeight = {small.height};\n")
    out.append(
        f"inline constexpr uint32_t kWordmarkRgb888 = 0x{color[0]:02X}{color[1]:02X}{color[2]:02X};\n\n"
    )
    out.append(f"inline constexpr uint8_t kWordmarkAlpha[{len(data)}] = {{\n")
    out.append(emit_bytes(data))
    out.append("};\n\n}  // namespace daoa\n")
    (OUT_DIR / "wordmark_data.h").write_text("".join(out), encoding="utf-8")
    print(f"wordmark : {small.width}×{small.height}, #{color[0]:02X}{color[1]:02X}{color[2]:02X}")


def write_ds_assets() -> None:
    out = [GEN_NOTE]
    out.append("// Assets bitmap du design system daoa-mini-design : masques 8 bits\n")
    out.append("// (canal alpha = couverture, teintés à l'exécution) et fond promo\n")
    out.append("// précomposé sur noir en RGB565.\n")
    out.append("#pragma once\n\n#include <cstdint>\n\nnamespace daoa {\n\n")
    out.append(
        "struct MaskAsset {\n"
        "    int width;\n"
        "    int height;\n"
        "    const uint8_t* alpha;\n"
        "};\n\n"
    )

    for filename, name in MASKS:
        im = Image.open(BITMAP_DIR / filename)
        data = alpha_mask(im)
        out.append(f"inline constexpr uint8_t kMask{name}Alpha[{len(data)}] = {{\n")
        out.append(emit_bytes(data))
        out.append("};\n")
        out.append(
            f"inline constexpr MaskAsset kMask{name}{{{im.width}, {im.height}, kMask{name}Alpha}};\n\n"
        )
        print(f"masque {filename} : {im.width}×{im.height}")

    # Fond promo : aplat précomposé sur noir, stocké en RGB565 natif.
    bg = Image.open(BITMAP_DIR / "promo-background-240x135.png").convert("RGB")
    assert bg.size == (240, 135), f"fond promo : {bg.size}"
    words = []
    for r, g, b in bg.getdata():
        words.append(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))
    out.append(f"inline constexpr int kPromoBgWidth = {bg.width};\n")
    out.append(f"inline constexpr int kPromoBgHeight = {bg.height};\n")
    out.append(f"inline constexpr uint16_t kPromoBg565[{len(words)}] = {{\n")
    for i in range(0, len(words), 12):
        out.append("    " + ",".join(f"0x{w:04X}" for w in words[i : i + 12]) + ",\n")
    out.append("};\n\n}  // namespace daoa\n")
    (OUT_DIR / "ds_assets.h").write_text("".join(out), encoding="utf-8")
    print(f"fond promo : {bg.width}×{bg.height} ({len(words) * 2} octets)")


if __name__ == "__main__":
    if not WORDMARK_SRC.is_file():
        raise SystemExit(
            f"Lettermark introuvable : {WORDMARK_SRC}\n"
            "Renseigner $DAOA_IOS_PATH (dépôt Daoa 2 iOS, hors de ce dépôt)."
        )
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    write_wordmark()
    write_ds_assets()
