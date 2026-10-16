***English** · [Français](README.fr.md)*

# Daoa Mini

**A line-by-line Yi Jing casting, on a pocket computer.**
M5Stack Cardputer ADV · ESP32-S3 · 240 × 135 display · fully offline.

![Daoa Mini — the place is listening](docs/m5burner/01-hero.png)

> ### The place is listening.

Daoa Mini is a small promotional object for the [**Daoa**](https://daoa.app)
mobile app (iPhone / Android): you cast six lines by the traditional
three-coin method, the device composes the figure, its complementary
figure, and gives you the text — in French or English. No network, no
account, no AI. The randomness comes from the ESP32's hardware generator.

Its distinctive feature: before every casting, the device **listens to the
place**.

![Home, casting, result, promo](docs/images/hero.png)

---

## The place is listening

<img src="docs/images/champ-vivant.gif" width="480" alt="The ambient field moving during a casting">

A **passive WiFi scan** — never connected, radio switched off the moment
it ends, no SSID or BSSID collected, nothing stored or transmitted —
becomes a field of particles: one per network in range, its position
derived from the channel, its brightness from the signal strength. They
drift, breathe, emit concentric ripples, and flare when a line is cast.

That field **weights the casting**, in a bounded and documented way:

| Reading | Derived from | Effect |
|---|---|---|
| **density** | network count + mean energy | leans yin or yang, **± 5 % at most** |
| **agitation** | channel spread + signal variance | makes changing lines likelier, **+ 4 % at most** |

The casting stays real: the hardware TRNG remains the only source of
randomness, and the user never chooses anything. The feature is **on by
default**, switchable off in the settings (`s`), and fully inspectable
with the `w` key:

![Technical view of the field](docs/images/champ-technique.png)

> Networks, channels, dBm, density, agitation, β, γ, and the resulting
> probabilities of the four line values. Transparency is the condition of
> the rule — see [docs/03](docs/03_MECANIQUE_TIRAGE.md) (French).

---

## The screens

| | |
|:--:|:--:|
| ![Home](docs/images/accueil.png) | ![Reading the place](docs/images/lecture-du-lieu.png) |
| **Home** — the lettermark, the breathing prompt | **Reading the place** — ~1.4 s: the scan time becomes the ritual |
| ![Casting](docs/images/tirage.png) | ![Result](docs/images/resultat.png) |
| **Casting** — six lines bottom-up, cinnabar marks on changing ones | **Result** — glyph, name, gloss, mini-figure, two pages |
| ![Reading](docs/images/lecture-texte.png) | ![Structure](docs/images/structure.png) |
| **Reading** — portrait, Judgment, Image, the six lines' meanings | **Structure** — the two trigrams, derived from the lines |
| ![Settings](docs/images/reglages.png) | ![Promo](docs/images/promo.png) |
| **Settings** — place reading, sounds, volume; persisted to NVS | **Promo** — QR code to the full app |

---

## Getting around

| Key | Action |
|---|---|
| `space` | begin, cast a line, continue |
| `←` `→` | first figure ↔ complementary figure |
| `↓` | full text of the figure; then pages; then structure |
| `↑` | go back through the pages |
| `l` | French ↔ English |
| `h` | built-in help (7 sections) |
| `s` | settings (place reading, sounds, volume) |
| `w` | technical view of the field |
| `esc` | back |

Sounds are soft-decaying PCM sinusoids, never a square buzzer: the six
lines walk up a **pentatonic scale** (A5 → A6), a fifth above marks the
changing lines, and a descending fifth resolves when the result appears.

<details>
<summary>Why that scale sits two octaves higher than it "should"</summary>

The scale was first written where it reads naturally on paper — A3 to A4,
220 to 440 Hz. On the device, the casting was **completely silent** while
the interface ticks were perfectly audible.

Nothing wrong with the code: a 1 W micro speaker in a pocket-sized
enclosure has its resonance around 1 kHz and rolls off sharply below
~400 Hz. The notes were being played, faithfully, into a band the hardware
cannot reproduce. Transposing the whole scale up two octaves — musical
relationships untouched, register moved into the speaker's efficient band —
fixed it. Worth remembering before designing any audio for this class of
device.

</details>

---

## The mechanics

Six lines, cast **bottom to top**. Each is a 6, 7, 8 or 9:

| Value | Nature | Line | Changing |
|:--:|---|---|:--:|
| 6 | old yin | broken, marked with a cross | → yang |
| 7 | young yang | solid | — |
| 8 | young yin | broken | — |
| 9 | old yang | solid, marked with a circle | → yin |

**Traditional three-coin probabilities**: 1/8, 3/8, 3/8, 1/8 — a
deliberate departure from Daoa v1, which drew uniformly. Changing lines
become rare events again, and therefore readable.

The six lines reduced to yin/yang give 6 bits → a **King Wen** number
through a lookup table (Javary/Faure ordering). Transforming the changing
lines yields the **complementary figure**: the one the situation tends
toward.

<details>
<summary><b>The detail that nearly caught us out</b> — why we don't bias the coins</summary>

The natural way to weight the casting was to bias the coins themselves:
P(heads) = 0.5 + ε. It is **mathematically useless**.

In the three-coin method a line is yang when the number of heads is
**odd** (7 = one head, 9 = three heads). And the parity of a binomial
draw is remarkably insensitive to bias:

$$P(\text{odd}) = \frac{1 - (1-2p)^3}{2}$$

Substituting $p = 0.5 + \varepsilon$, everything cancels down to **third
order**: $P(\text{yang}) = 0.5 + 4\varepsilon^3$. With a 5 % coin bias the
real effect would be **0.05 %** — a thousand times too small to matter.

So the implemented construction acts at the line level, as two independent
Bernoulli trials, strictly equivalent to the tradition when unweighted:

```
P(yang)     = 0.5  + β     β = (density − 0.5) × 0.1   ∈ [−0.05, +0.05]
P(changing) = 0.25 + γ     γ = agitation × 0.04        ∈ [0, 0.04]
```

At β = γ = 0 you get exactly 1/8, 3/8, 3/8, 1/8 back. Thresholds are
compared against 10-bit fields of the TRNG (512/1024 and 256/1024: exact,
no modulo bias). One test asserts the neutrality, another that the effect
stays bounded at the caps.

</details>

---

## The hardware

| | |
|---|---|
| **MCU** | ESP32-S3FN8 (StampS3A), dual-core Xtensa LX7 @ 240 MHz |
| **Memory** | 8 MB flash, 512 KB SRAM, **no PSRAM** |
| **Display** | ST7789V2, 1.14″, **240 × 135** — about 25 × 14 mm at ~245 ppi |
| **Keyboard** | 56 keys, **TCA8418** I²C controller |
| **Audio** | ES8311 codec + NS4150B + 1 W speaker |
| **Also** | BMI270 IMU, IR, microSD, 1500 mAh battery (1750 mAh on units made before July 2026) |

Firmware footprint: **3.47 MB of flash** (41 % of 8 MB) and **50.9 KB of
RAM** (15.5 %). The full-screen framebuffer is a 16-bit, 64.8 KB sprite in
SRAM: everything is drawn off-screen and pushed in one go — no flicker.

---

## Build, simulate, flash

```bash
brew install platformio sdl2
```

```bash
# Pure-logic tests (20 cases, no hardware needed)
pio test -e test-native

# macOS simulator: SDL window, 240 × 135 at ×3
pio run -e sim && ./.pio/build/sim/program

# Firmware, with the Cardputer plugged in over USB
pio run -e cardputer-adv -t upload
```

The distributable image is `firmware.factory.bin` (bootloader + partition
table + app, written at offset 0) — not `firmware.bin`, which is the
application alone.

The two generator scripts locate their private sources through
`$DAOA_V1_PATH` and `$DAOA_IOS_PATH`; no machine path is written into the
repository. Without them the scripts stop with a clear message, and the
generated headers they produce are not versioned — see below.

Three PlatformIO environments: the firmware (Arduino-ESP32 3.x through
pioarduino), the **SDL simulator**, and the **native tests**.

### The simulator

All rendering goes through the `lgfx::` API — never through the hardware —
so the **same code** draws on the device and on the Mac. That is what makes
visual iteration bearable: no 30-second flash cycle to nudge something by
2 px. It also produces deterministic captures:

```bash
./.pio/build/sim/program --shot   <dir>   # 17 screens, fixed randomness
./.pio/build/sim/program --frames <dir>   # 60 frames of the living field
./.pio/build/sim/program --seed   <n>     # replay one specific casting
./.pio/build/sim/program --fonttest <dir> # glyph coverage sheet
```

Every image in this README comes out of it.

---

## Architecture

```
src/
  core/       pure logic — C++17, zero Arduino/M5 dependency, natively tested
    casting   the casting state machine, King Wen table
    ambient   place reading → bounded weights
  ambient/    hardware boundary: real WiFi scan / simulated scan
  sound/      PCM synthesis of the interface sounds
  ui/         design-system tokens, LovyanGFX components, screens
  content/    generated corpus (not versioned — see below)
  sim/        SDL entry point — captures, frames, seeds
  main.cpp    device entry point — TCA8418 keyboard mapping
```

Randomness is **injected** (`uint32_t (*)()`): `esp_random()` on the
device, a deterministic PRNG in tests and in the simulator. Same for the
WiFi scan and for persistence — `core/` only ever sees interfaces, which
is what makes it fully testable on a desktop machine.

### The design system

The interface follows a dedicated micro design system, where every
dimension is a `constexpr` in [`src/ui/daoa_tokens.h`](src/ui/daoa_tokens.h)
and every component a render function in
[`src/ui/components.cpp`](src/ui/components.cpp):

- **135 isn't divisible by 4** — so there is no vertical grid, but three
  fixed bands instead: header 18, content 99, footer 18.
- A 228 px column = **38 characters** at 6 px advance; three type sizes
  (12 / 16 / 24), leading = height + 2.
- Yin/yang lines in three sizes, thickness/width ratio held at 1:12.
- Five colours, not one more. **Paper** for what speaks of the casting,
  **stone** for what speaks of the interface, **cinnabar** reserved for
  changing-line marks, **terracotta** reserved for the brand.
- No shadows, no borders, no cards: the bands read through emptiness. At
  245 ppi on black, a 1 px rule draws more attention than the text it
  separates.

### The font expedition

M5GFX's DejaVu fonts are **pure ASCII**: no accents at all, as discovered
while rendering « Créatif ». After measuring in the simulator, body text
moved to **efontJA** (which covers accented Latin) and the glyphs to
**efontCN 24 bold** — except 遯 (33), 夬 (43) and 姤 (44), missing from the
CN font, which fall back to efontJA. A dedicated VLW font for the exact 64
glyphs is on the roadmap.

---

## What this repository does not contain

The repository is public and holds the **engine** — not the Daoa brand or
content. Excluded, and purged from history:

- design deliverables and brand assets;
- the **generated** headers that encode brand or content: the bilingual
  corpus of the 64 figures, and the bitmap masks.

`src/core/kingwen_table.h` stays public: it is mathematical heritage, not
Daoa content.

**An accepted consequence: a fresh public clone will not build as-is.**
The scripts `scripts/extract_content.py` and `scripts/convert_assets.py`
regenerate those headers from private sources. The code, the architecture
and the mechanics remain entirely readable — which is what's worth
anything to someone passing by.

---

## Documentation

The project documentation is in French.

| | |
|---|---|
| [CLAUDE.md](CLAUDE.md) | project doctrine, guardrails, technical decisions |
| [docs/00](docs/00_ANALYSE_BESOINS.md) | initial analysis: hardware, libraries, tooling |
| [docs/01](docs/01_DECISIONS.md) | log of ratified decisions |
| [docs/02](docs/02_ARCHITECTURE.md) | tree, build environments, content pipeline |
| [docs/03](docs/03_MECANIQUE_TIRAGE.md) | **casting specification** — source of truth |
| [docs/04](docs/04_UX_RENDU.md) | screens, visual direction, fonts, animation |
| [docs/05](docs/05_ROADMAP.md) | phases and current state |

---

## Licence

The **code** in this repository is released under the [MIT licence](LICENSE)
— take it, learn from it, build on it.

What the licence does *not* cover, because it is not in the repository:
the **Daoa brand** (lettermark, seal, visual identity) and the **Yi Jing
corpus** written for Daoa. Those stay proprietary, and are generated
locally from private sources.

---

## About the texts

The texts for the 64 figures — portrait, Judgment, Image, and the meaning
of the six lines — are **Daoa's own wording**, written in faithfulness to
public-domain translations: James Legge (1882) for English, Charles de
Harlez (1889) for French. Nothing is copied word for word from a protected
translation.

The Yi Jing is presented here as a text for reflection, not as an oracle.
The device describes; it does not predict.
