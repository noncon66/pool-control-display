#pragma once

#include <stdint.h>

enum class PoolMode : uint8_t
{
    // Diese Zahlenwerte müssen mit den von Loxone gesendeten Werten
    // übereinstimmen. Heizen ist kein Betriebsmodus, sondern wird separat über
    // PoolState::isHeating abgebildet.
    Off = 0,
    Auto = 1,
    Manual = 2
};

struct PoolState
{
    // Nach dieser Zeit ohne gültige Loxone-Meldung gelten vorhandene Daten als
    // veraltet. Der Wert kann später bei Bedarf zentral angepasst werden.
    static constexpr uint32_t STATUS_STALE_AFTER_MS = 60000;

    // Diese Struktur ist nur ein lokales Abbild der zuletzt von Loxone
    // bestätigten Werte. Sie enthält keinerlei Pool-Steuerungslogik.
    float waterTemperature = 0.0f;
    float targetTemperature = 0.0f;

    bool filterPump = false;
    bool heatingPump = false;
    bool heatingAllowed = false;
    bool isHeating = false;

    PoolMode mode = PoolMode::Off;

    // Ein Standardwert allein zeigt nicht, ob bereits eine MQTT-Nachricht
    // angekommen ist. Deshalb besitzt jeder Wert ein passendes "has..."-Feld.
    // Solange dieses false ist, muss die Oberfläche "unbekannt" anzeigen.
    bool hasWaterTemperature = false;
    bool hasTargetTemperature = false;
    bool hasFilterPump = false;
    bool hasHeatingPump = false;
    bool hasHeatingAllowed = false;
    bool hasIsHeating = false;
    bool hasMode = false;

    // Zeitpunkt der letzten gültigen Statusmeldung. millis() zählt die
    // Millisekunden seit dem Start des ESP32.
    uint32_t lastStatusUpdateAt = 0;

    // Diese beiden Werte beschreiben nur die Verbindung des Panels.
    bool wifiConnected = false;
    bool mqttConnected = false;

    // true bedeutet nur, dass wenigstens ein bestätigter Loxone-Wert vorliegt.
    bool hasAnyStatus() const
    {
        return hasWaterTemperature ||
               hasTargetTemperature ||
               hasFilterPump ||
               hasHeatingPump ||
               hasHeatingAllowed ||
               hasIsHeating ||
               hasMode;
    }

    // Vorzeichenlose Subtraktion behandelt den Überlauf von millis() korrekt.
    // "Aktuell" bedeutet nicht "sicher", sondern beschreibt nur das Datenalter.
    bool isStatusFresh(uint32_t now) const
    {
        return hasAnyStatus() &&
               (now - lastStatusUpdateAt <= STATUS_STALE_AFTER_MS);
    }

    // Diese Information dient nur dazu, das Bedienelement im Panel
    // freizugeben. Loxone muss jeden Filterpumpenbefehl trotzdem selbst prüfen.
    bool isManualModeConfirmed() const
    {
        return hasMode && mode == PoolMode::Manual;
    }
};
