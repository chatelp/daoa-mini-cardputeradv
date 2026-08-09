#!/usr/bin/env python3
"""Images de publication M5Burner pour Daoa Mini.

La vignette du catalogue est une boîte 4:3 d'environ 170 × 135 px : tout
doit survivre à une réduction ×7. Palette du design system ; rythme des
traits repris de l'affiche de l'app Daoa (espace ≈ épaisseur, extrémités
arrondies — contrairement à l'appareil, où le rayon 0 est imposé par la
résolution).

Sortie : docs/m5burner/ — l'affiche puis les écrans réels, dans l'ordre du
carrousel. Le lettermark vient des sources privées (voir CLAUDE.md).

Usage : python3 scripts/make_store_images.py
"""

import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "docs/m5burner"
SHOTS = ROOT / "docs/images"
OUT.mkdir(parents=True, exist_ok=True)
WORDMARK = Path("$DAOA_IOS_PATH/Daoa/Resources/Assets.xcassets/"
                "WordmarkSplash.imageset/dark.png")

W, H = 1200, 900
BLACK, PAPER = (0, 0, 0), (236, 228, 210)
STONE, CINNABAR, TERRACOTTA = (148, 139, 122), (198, 91, 66), (201, 126, 88)

SERIF = "/System/Library/Fonts/Supplemental/Georgia.ttf"
SANS = "/System/Library/Fonts/Avenir Next.ttc"

JIJI = [True, False, True, False, True, False]   # 63 既濟 — alternance parfaite


def font(p, s, i=None):
    return ImageFont.truetype(p, s) if i is None else ImageFont.truetype(p, s, index=i)


def tw(d, s, f):
    b = d.textbbox((0, 0), s, font=f)
    return b[2] - b[0]


def centered(d, s, f, cx, y, fill):
    d.text((cx - tw(d, s, f) // 2, y), s, font=f, fill=fill)


def hex_height(thick, gap):
    return 6 * thick + 5 * gap


def draw_hexagram(d, lines, cx, top, width, thick, gap, mutating=None, color=PAPER):
    """lines : 6 booléens bas→haut, True = yang. Extrémités arrondies."""
    split = int(thick * 2.0)
    seg = (width - split) // 2
    r = int(thick * 0.30)
    for i, yang in enumerate(lines):
        y = top + (5 - i) * (thick + gap)
        x0 = cx - width // 2
        if yang:
            d.rounded_rectangle([x0, y, x0 + width, y + thick], radius=r, fill=color)
        else:
            d.rounded_rectangle([x0, y, x0 + seg, y + thick], radius=r, fill=color)
            d.rounded_rectangle([x0 + width - seg, y, x0 + width, y + thick],
                                radius=r, fill=color)
        if mutating and i in mutating:
            mr = int(thick * 0.85)
            mx = x0 + width + int(thick * 2.4)
            my = y + thick // 2
            d.ellipse([mx - mr, my - mr, mx + mr, my + mr],
                      outline=CINNABAR, width=max(4, int(thick * 0.24)))


def wordmark(img, cx, top, width, color=TERRACOTTA):
    wm = Image.open(WORDMARK).convert("RGBA")
    wm = wm.crop(wm.getchannel("A").getbbox())
    h = round(wm.height * width / wm.width)
    wm = wm.resize((width, h), Image.LANCZOS)
    tint = Image.new("RGBA", wm.size, color + (255,))
    tint.putalpha(wm.getchannel("A"))
    img.paste(tint, (cx - width // 2, top), tint)
    return h


def ambient_field(d, seed=12345, n=26, keep_out_y=None):
    st = seed

    def rnd():
        nonlocal st
        st ^= (st << 13) & 0xFFFFFFFF
        st ^= st >> 17
        st ^= (st << 5) & 0xFFFFFFFF
        return st

    limit = keep_out_y if keep_out_y else H - 40
    pts = []
    for _ in range(n):
        x, y = 40 + rnd() % (W - 80), 40 + rnd() % (limit - 40)
        lum = 0.3 + (rnd() % 100) / 100 * 0.7
        pts.append((x, y, lum))
    for x, y, lum in pts:
        d.ellipse([x - 13, y - 13, x + 13, y + 13],
                  fill=tuple(int(c * lum * 0.16) for c in STONE))
        d.ellipse([x - 5, y - 5, x + 5, y + 5],
                  fill=tuple(int(c * lum * 0.85) for c in STONE))
    # Deux ondes discrètes, dans les coins hauts : elles ne croisent jamais
    # la marque, qui doit rester le seul élément net du bas.
    # Jamais près de la marque de mutation (haut droite) ni du lettermark
    # (bas centre) : le cinabre doit rester le seul cercle qui attire l'œil.
    for (cx, cy), rad in (((150, 200), 125), ((1085, 645), 140)):
        d.ellipse([cx - rad, cy - rad, cx + rad, cy + rad],
                  outline=tuple(int(c * 0.26) for c in STONE), width=4)


# ---------------------------------------------------------------- concepts

def build(name, *, field=False, thick=54, gap=54, width=650, wm_w=320,
          mutating={4}, tagline=None, bleed=False):
    img = Image.new("RGB", (W, H), BLACK)
    d = ImageDraw.Draw(img)
    hh = hex_height(thick, gap)
    wm_h = round(672 * wm_w / 1600)
    tag_h = 62 if tagline else 0
    block = hh + 104 + wm_h + tag_h
    y = (H - block) // 2
    if field:
        ambient_field(d, n=20, keep_out_y=y + hh + 70)
    draw_hexagram(d, JIJI, W // 2, y, width, thick, gap, mutating=mutating)
    y += hh + 104
    y += wordmark(img, W // 2, y, wm_w)
    if tagline:
        y += 30
        centered(d, tagline, font(SERIF, 44), W // 2, y, STONE)
    img.save(OUT / f"{name}.png")


def hero():
    """L'affiche du carrousel : la figure, le champ, la marque, la ligne."""
    build("01-hero", field=True, thick=50, gap=50, width=610, wm_w=300,
          tagline="The place is listening")


def screen(src, dst, scale=4):
    """Un écran réel de l'appareil, au centre d'un cadre 4:3 noir.

    Les sources de docs/images sont en ×3 : on revient au natif 240 × 135
    puis on agrandit d'un facteur entier, sinon les pixels bavent."""
    im = Image.open(SHOTS / src).convert("RGB")
    im = im.resize((240, 135), Image.NEAREST)
    im = im.resize((240 * scale, 135 * scale), Image.NEAREST)
    canvas = Image.new("RGB", (W, H), BLACK)
    canvas.paste(im, ((W - im.width) // 2, (H - im.height) // 2))
    canvas.save(OUT / dst)


for fn in (hero,):
    fn()
# Le carrousel : l'affiche, puis la preuve. La vue technique en troisième
# position — c'est elle qui parle au public de M5Burner.
screen("tirage.png", "02-casting.png")
screen("champ-technique.png", "03-field.png")
screen("resultat.png", "04-result.png")
screen("lecture-texte.png", "05-reading.png")
screen("reglages.png", "06-settings.png")
print("ok :", ", ".join(p.name for p in sorted(OUT.glob("*.png"))))
