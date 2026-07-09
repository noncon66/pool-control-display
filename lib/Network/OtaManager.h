#pragma once

#include <Arduino.h>

class WifiManager;

// Kapselt Firmware-Updates über WLAN. OTA ist standardmäßig deaktiviert und
// wird erst gestartet, wenn WLAN verbunden und ein Passwort konfiguriert ist.
class OtaManager
{
public:
    void begin(WifiManager& wifi);
    void loop();

    bool isReady() const { return _ready; }
    bool isUpdating() const { return _updating; }
    uint8_t progressPercent() const { return _progressPercent; }

private:
    WifiManager* _wifi = nullptr;
    bool _ready = false;
    bool _configurationErrorReported = false;
    bool _updating = false;
    uint8_t _progressPercent = 0;

    void start();
};
