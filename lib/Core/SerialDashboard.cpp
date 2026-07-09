#include "SerialDashboard.h"

#include <Arduino.h>
#include "WifiManager.h"
#include "MqttManager.h"

void SerialDashboard::begin()
{
    Serial.println("[Dashboard] ready");
}

void SerialDashboard::renderFull(const PoolState& state, const WifiManager& wifi, const MqttManager& mqtt)
{
    printHeader("POOL CONTROL DISPLAY");

    // Ein gespeicherter Wert wird erst angezeigt, wenn sein has...-Feld den
    // Empfang von Loxone bestätigt. So wirken Standardwerte nicht wie Messwerte.
    if (state.hasWaterTemperature) Serial.printf("Water temp      : %.1f C\n", state.waterTemperature);
    else                           Serial.println("Water temp      : UNKNOWN");
    if (state.hasTargetTemperature) Serial.printf("Target temp     : %.1f C\n", state.targetTemperature);
    else                            Serial.println("Target temp     : UNKNOWN");
    Serial.printf("Filter pump     : %s\n", state.hasFilterPump ? (state.filterPump ? "ON" : "OFF") : "UNKNOWN");
    Serial.printf("Heating pump    : %s\n", state.hasHeatingPump ? (state.heatingPump ? "ON" : "OFF") : "UNKNOWN");
    Serial.printf("Heating allowed : %s\n", state.hasHeatingAllowed ? (state.heatingAllowed ? "YES" : "NO") : "UNKNOWN");
    Serial.printf("Pool heating    : %s\n", state.hasIsHeating ? (state.isHeating ? "YES" : "NO") : "UNKNOWN");
    Serial.printf("Mode            : %s\n", state.hasMode ? modeToString(state.mode) : "UNKNOWN");

    Serial.printf("WiFi connected  : %s\n", wifi.isConnected() ? "YES" : "NO");
    if (wifi.isConnected())
    {
        Serial.printf("IP address      : %s\n", wifi.ipAddress());
    }

    Serial.printf("MQTT connected  : %s\n", mqtt.isConnected() ? "YES" : "NO");
    Serial.printf("Loxone data     : %s\n",
        !state.hasAnyStatus() ? "UNKNOWN" :
        (state.isStatusFresh(millis()) ? "CURRENT" : "STALE"));
    Serial.println();
}

void SerialDashboard::renderDiff(const PoolState& oldState, const PoolState& newState, const WifiManager& wifi, const MqttManager& mqtt)
{
    printHeader("STATE UPDATE");

    // Wert und Gültigkeitsfeld werden verglichen. Ein erstmals bestätigter Wert
    // von 0.0 muss ausgegeben werden, obwohl der Speicher mit 0.0 begann.
    if (oldState.waterTemperature != newState.waterTemperature ||
        oldState.hasWaterTemperature != newState.hasWaterTemperature)
    {
        if (newState.hasWaterTemperature) Serial.printf("Water temp      : %.1f C\n", newState.waterTemperature);
        else                              Serial.println("Water temp      : UNKNOWN");
    }

    if (oldState.targetTemperature != newState.targetTemperature ||
        oldState.hasTargetTemperature != newState.hasTargetTemperature)
    {
        if (newState.hasTargetTemperature) Serial.printf("Target temp     : %.1f C\n", newState.targetTemperature);
        else                               Serial.println("Target temp     : UNKNOWN");
    }

    if (oldState.filterPump != newState.filterPump ||
        oldState.hasFilterPump != newState.hasFilterPump)
    {
        Serial.printf("Filter pump     : %s\n",
            newState.hasFilterPump ? (newState.filterPump ? "ON" : "OFF") : "UNKNOWN");
    }

    if (oldState.heatingPump != newState.heatingPump ||
        oldState.hasHeatingPump != newState.hasHeatingPump)
    {
        Serial.printf("Heating pump    : %s\n",
            newState.hasHeatingPump ? (newState.heatingPump ? "ON" : "OFF") : "UNKNOWN");
    }

    if (oldState.heatingAllowed != newState.heatingAllowed ||
        oldState.hasHeatingAllowed != newState.hasHeatingAllowed)
    {
        Serial.printf("Heating allowed : %s\n",
            newState.hasHeatingAllowed ? (newState.heatingAllowed ? "YES" : "NO") : "UNKNOWN");
    }

    if (oldState.isHeating != newState.isHeating ||
        oldState.hasIsHeating != newState.hasIsHeating)
    {
        Serial.printf("Pool heating    : %s\n",
            newState.hasIsHeating ? (newState.isHeating ? "YES" : "NO") : "UNKNOWN");
    }

    if (oldState.mode != newState.mode ||
        oldState.hasMode != newState.hasMode)
    {
        Serial.printf("Mode            : %s\n",
            newState.hasMode ? modeToString(newState.mode) : "UNKNOWN");
    }

    if (oldState.wifiConnected != newState.wifiConnected)
    {
        Serial.printf("WiFi connected  : %s -> %s\n",
            oldState.wifiConnected ? "YES" : "NO",
            newState.wifiConnected ? "YES" : "NO");

        if (wifi.isConnected())
        {
            Serial.printf("IP address      : %s\n", wifi.ipAddress());
        }
    }

    if (oldState.mqttConnected != newState.mqttConnected)
    {
        Serial.printf("MQTT connected  : %s -> %s\n",
            oldState.mqttConnected ? "YES" : "NO",
            newState.mqttConnected ? "YES" : "NO");
    }

    Serial.println();
}

const char* SerialDashboard::modeToString(PoolMode mode) const
{
    // Stringliterale bleiben während der gesamten Programmlaufzeit bestehen
    // und können daher sicher zurückgegeben werden.
    switch (mode)
    {
        case PoolMode::Off:     return "Off";
        case PoolMode::Auto:    return "Auto";
        case PoolMode::Manual:  return "Manual";
        default:                return "Unknown";
    }
}

void SerialDashboard::printHeader(const char* title) const
{
    Serial.println("========================================");
    Serial.println(title);
    Serial.println("========================================");
}
