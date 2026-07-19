#pragma once

#include <stdint.h>

enum class PoolMode : uint8_t
{
    // Diese Zahlenwerte müssen mit den von Loxone gesendeten Werten
    // übereinstimmen. Heizen ist kein eigener Betriebsmodus.
    Auto = 1,
    Manual = 2,
    Off = 3
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

    // Zeitpunkt der letzten gültigen Statusmeldung. Dieser gemeinsame
    // Zeitstempel zeigt, ob Loxone grundsätzlich noch Daten sendet.
    uint32_t lastStatusUpdateAt = 0;

    // Eigene Zeitstempel bleiben für Diagnose und eine mögliche spätere
    // Heartbeat-Strategie erhalten. Die aktuelle Bedienpolitik verwendet
    // retained Werte bei bestehender MQTT-Verbindung ohne Altersgrenze.
    uint32_t lastTargetTemperatureUpdateAt = 0;
    uint32_t lastFilterPumpUpdateAt = 0;
    uint32_t lastModeUpdateAt = 0;

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
               hasMode;
    }

    // Vorzeichenlose Subtraktion behandelt den Überlauf von millis() korrekt.
    // "Aktuell" bedeutet nicht "sicher", sondern beschreibt nur das Datenalter.
    bool isStatusFresh(uint32_t now) const
    {
        return isValueFresh(hasAnyStatus(), lastStatusUpdateAt, now);
    }

    bool isTargetTemperatureFresh(uint32_t now) const
    {
        return isValueFresh(
            hasTargetTemperature,
            lastTargetTemperatureUpdateAt,
            now);
    }

    bool isFilterPumpFresh(uint32_t now) const
    {
        return isValueFresh(hasFilterPump, lastFilterPumpUpdateAt, now);
    }

    bool isModeFresh(uint32_t now) const
    {
        return isValueFresh(hasMode, lastModeUpdateAt, now);
    }

    // Diese Information dient nur dazu, das Bedienelement im Panel
    // freizugeben. Loxone muss jeden Filterpumpenbefehl trotzdem selbst prüfen.
    bool isManualModeConfirmed() const
    {
        return hasMode && mode == PoolMode::Manual;
    }

private:
    // Vorzeichenlose Subtraktion behandelt den Überlauf von millis() korrekt.
    static bool isValueFresh(bool hasValue, uint32_t updatedAt, uint32_t now)
    {
        return hasValue && (now - updatedAt <= STATUS_STALE_AFTER_MS);
    }
};
