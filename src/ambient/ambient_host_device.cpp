// Hôte réel de la lecture du lieu : scan WiFi passif de l'ESP32-S3 et
// réglage persistant en NVS. Jamais de connexion ; la radio est coupée
// sitôt les résultats copiés. Rien n'est stocké ni transmis.
#ifndef DAOA_SIM

#include <Preferences.h>
#include <WiFi.h>

#include "ambient_host.h"

namespace daoa {

namespace {

Preferences g_prefs;

void startScanDevice(uint32_t) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();  // jamais associé à un réseau
    WiFi.scanNetworks(/*async=*/true, /*show_hidden=*/true);
}

int pollScanDevice(uint32_t, ApReading* out, int max) {
    const int16_t n = WiFi.scanComplete();
    if (n == WIFI_SCAN_RUNNING) return -1;
    int count = 0;
    if (n > 0) {
        for (int i = 0; i < n && count < max; ++i) {
            out[count].rssi = static_cast<int8_t>(WiFi.RSSI(i));
            out[count].channel = static_cast<uint8_t>(WiFi.channel(i));
            ++count;
        }
    }
    WiFi.scanDelete();
    WiFi.mode(WIFI_OFF);  // hors-ligne sitôt la lecture faite
    return count;
}

bool loadEnabledDevice() {
    g_prefs.begin("daoa", true);
    const bool enabled = g_prefs.getBool("ambient", true);
    g_prefs.end();
    return enabled;
}

void saveEnabledDevice(bool enabled) {
    g_prefs.begin("daoa", false);
    g_prefs.putBool("ambient", enabled);
    g_prefs.end();
}

bool loadSoundDevice() {
    g_prefs.begin("daoa", true);
    const bool enabled = g_prefs.getBool("sound", true);
    g_prefs.end();
    return enabled;
}

void saveSoundDevice(bool enabled) {
    g_prefs.begin("daoa", false);
    g_prefs.putBool("sound", enabled);
    g_prefs.end();
}

uint8_t loadVolumeDevice() {
    g_prefs.begin("daoa", true);
    const uint8_t level = g_prefs.getUChar("volume", 1);
    g_prefs.end();
    return level > 2 ? 1 : level;
}

void saveVolumeDevice(uint8_t level) {
    g_prefs.begin("daoa", false);
    g_prefs.putUChar("volume", level);
    g_prefs.end();
}

}  // namespace

extern const AmbientHost kDeviceAmbientHost;
const AmbientHost kDeviceAmbientHost{startScanDevice,   pollScanDevice,
                                     loadEnabledDevice, saveEnabledDevice,
                                     loadSoundDevice,   saveSoundDevice,
                                     loadVolumeDevice,  saveVolumeDevice};

}  // namespace daoa

#endif  // !DAOA_SIM
