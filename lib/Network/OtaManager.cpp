#include "OtaManager.h"

#include <ArduinoOTA.h>

#include "PoolConfig.h"
#include "WifiManager.h"

void OtaManager::begin(WifiManager& wifi)
{
    _wifi = &wifi;

    if (!OTA_ENABLED)
    {
        Serial.println("[OTA] disabled");
    }
}

void OtaManager::loop()
{
    if (!OTA_ENABLED || _ready)
    {
        if (_ready)
        {
            // Eingehende OTA-Anfragen müssen regelmäßig verarbeitet werden.
            ArduinoOTA.handle();
        }
        return;
    }

    if (!_wifi || !_wifi->isConnected())
    {
        return;
    }

    // Ein leeres Passwort würde ungeschützte Firmware-Updates ermöglichen.
    if (strlen(OTA_PASSWORD) == 0)
    {
        if (!_configurationErrorReported)
        {
            Serial.println("[OTA] enabled but OTA_PASSWORD is empty; OTA not started");
            _configurationErrorReported = true;
        }
        return;
    }

    start();
}

void OtaManager::start()
{
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([this]()
    {
        _updating = true;
        _progressPercent = 0;
        Serial.println("[OTA] update started");
    });

    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total)
    {
        _progressPercent =
            total == 0 ? 0 : static_cast<uint8_t>((progress * 100U) / total);
        Serial.printf("[OTA] progress: %u%%\n", _progressPercent);
    });

    ArduinoOTA.onEnd([this]()
    {
        _progressPercent = 100;
        Serial.println("[OTA] update complete; rebooting");
    });

    ArduinoOTA.onError([this](ota_error_t error)
    {
        _updating = false;
        Serial.printf("[OTA] error: %u\n", static_cast<unsigned int>(error));
    });

    ArduinoOTA.begin();
    _ready = true;
    Serial.printf("[OTA] ready as '%s'\n", OTA_HOSTNAME);
}
