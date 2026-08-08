#!/usr/bin/env python3
"""Convertit les ressources de marque Daoa 2 en en-têtes C++ embarqués.

Source : le lettermark « daoa » de daoa-ios (WordmarkSplash, variante fond
sombre). On n'embarque qu'un masque de couverture 8 bits (alpha), teinté à
l'exécution — cela préserve l'anti-aliasing ET le fondu d'entrée de l'app,
pour 1 octet/pixel.

Sortie : src/ui/assets/wordmark_data.h (ne pas éditer à la main).

Usage : python3 scripts/convert_assets.py
"""

from pathlib import Path

from PIL import Image

SRC = Path(
    "$DAOA_IOS_PATH/Daoa/Resources/Assets.xcassets/"
    "WordmarkSplash.imageset/dark.png"
)
ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "src/ui/assets/wordmark_data.h"

TARGET_WIDTH = 190  # sur 240 px de large : marges ~25 px de chaque côté


def main() -> None:
    im = Image.open(SRC).convert("RGBA")

    # Recadrage sur le contenu réel (l'export a des marges transparentes).
    alpha = im.getchannel("A")
    bbox = alpha.getbbox()
    im = im.crop(bbox)

    # Couleur de marque : moyenne des pixels pleinement opaques.
    px = im.load()
    rs = gs = bs = n = 0
    for y in range(0, im.height, 7):
        for x in range(0, im.width, 7):
            r, g, b, a = px[x, y]
            if a > 250:
                rs, gs, bs, n = rs + r, gs + g, bs + b, n + 1
    color = (rs // n, gs // n, bs // n)

    h = round(im.height * TARGET_WIDTH / im.width)
    small = im.resize((TARGET_WIDTH, h), Image.LANCZOS).getchannel("A")

    data = small.tobytes()
    out = ["// GÉNÉRÉ par scripts/convert_assets.py — NE PAS ÉDITER À LA MAIN.\n"]
    out.append("// Lettermark « daoa » (Daoa 2, WordmarkSplash/dark) : masque de\n")
    out.append("// couverture 8 bits, à teinter à l'exécution.\n")
    out.append("#pragma once\n\n#include <cstdint>\n\nnamespace daoa {\n\n")
    out.append(f"inline constexpr int kWordmarkWidth = {small.width};\n")
    out.append(f"inline constexpr int kWordmarkHeight = {small.height};\n")
    out.append(
        f"inline constexpr uint32_t kWordmarkRgb888 = 0x{color[0]:02X}{color[1]:02X}{color[2]:02X};\n\n"
    )
    out.append(f"inline constexpr uint8_t kWordmarkAlpha[{len(data)}] = {{\n")
    for i in range(0, len(data), 24):
        out.append("    " + ",".join(str(b) for b in data[i : i + 24]) + ",\n")
    out.append("};\n\n}  // namespace daoa\n")
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text("".join(out), encoding="utf-8")
    print(f"OK : {small.width}×{small.height}, couleur #{color[0]:02X}{color[1]:02X}{color[2]:02X}")
    print(f"  → {OUT.relative_to(ROOT)} ({len(data)} octets de masque)")


if __name__ == "__main__":
    main()
