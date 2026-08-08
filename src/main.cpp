// Entrée appareil : M5Stack Cardputer ADV.
// Mapping clavier → daoa::Key ; tout le reste vit dans ui/app.cpp.
#include <M5Cardputer.h>
#include <esp_random.h>

#include "ui/app.h"

namespace {

uint32_t trng() { return esp_random(); }

daoa::App app(trng);

daoa::Key readKey() {
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) {
        return daoa::Key::kNone;
    }
    const auto state = M5Cardputer.Keyboard.keysState();
    if (state.enter) return daoa::Key::kAction;
    for (char c : state.word) {
        switch (c) {
            case ' ': return daoa::Key::kAction;
            case ',': return daoa::Key::kPrev;   // ←
            case '/': return daoa::Key::kNext;   // →
            case ';': return daoa::Key::kUp;     // ↑
            case '.': return daoa::Key::kDown;   // ↓
            case 'l': return daoa::Key::kLang;
            case 'h': return daoa::Key::kHelp;
            case '`': return daoa::Key::kReset;
            default: break;
        }
    }
    return daoa::Key::kNone;
}

}  // namespace

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(160);
}

void loop() {
    M5Cardputer.update();
    const daoa::Key k = readKey();
    if (k != daoa::Key::kNone) app.handleKey(k, millis());
    app.render(M5Cardputer.Display, millis());
    delay(8);
}
