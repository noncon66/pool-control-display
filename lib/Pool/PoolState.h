#pragma once

#include <stdint.h>

enum class PoolMode : uint8_t
{
    // Diese Zahlenwerte müssen mit den von Loxone gesendeten Werten
    // übereinstimmen. uint8_t hält den Speicherbedarf klein.
    Off = 0,
    Auto = 1,
    Manual = 2,
    Heating = 3
};

struct PoolState
{
    // Diese Struktur ist nur ein lokales Abbild der zuletzt von Loxone
    // bestätigten Werte. Sie enthält keinerlei Pool-Steuerungslogik.
    float waterTemperature = 0.0f;
    float targetTemperature = 0.0f;

    bool filterPump = false;
    bool heatingPump = false;
    bool heatingAllowed = false;

    PoolMode mode = PoolMode::Off;

    // Ein Standardwert allein zeigt nicht, ob bereits eine MQTT-Nachricht
    // angekommen ist. Deshalb besitzt jeder Wert ein passendes "has..."-Feld.
    // Solange dieses false ist, muss die Oberfläche "unbekannt" anzeigen.
    bool hasWaterTemperature = false;
    bool hasTargetTemperature = false;
    bool hasFilterPump = false;
    bool hasHeatingPump = false;
    bool hasHeatingAllowed = false;
    bool hasMode = false;

    // Zeitpunkt der letzten gültigen Statusmeldung. millis() zählt die
    // Millisekunden seit dem Start des ESP32.
    uint32_t lastStatusUpdateAt = 0;

    // Diese beiden Werte beschreiben nur die Verbindung des Panels.
    bool wifiConnected = false;
    bool mqttConnected = false;
};
