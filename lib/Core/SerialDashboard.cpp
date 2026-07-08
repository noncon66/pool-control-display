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

    Serial.printf("Water temp      : %.1f °C\n", state.waterTemperature);
    Serial.printf("Target temp     : %.1f °C\n", state.targetTemperature);
    Serial.printf("Filter pump     : %s\n", state.filterPump ? "ON" : "OFF");
    Serial.printf("Heating pump    : %s\n", state.heatingPump ? "ON" : "OFF");
    Serial.printf("Heating allowed : %s\n", state.heatingAllowed ? "YES" : "NO");
    Serial.printf("Mode            : %s\n", modeToString(state.mode));

    Serial.printf("WiFi connected  : %s\n", wifi.isConnected() ? "YES" : "NO");
    if (wifi.isConnected())
    {
        Serial.printf("IP address      : %s\n", wifi.ipAddress());
    }

    Serial.printf("MQTT connected  : %s\n", mqtt.isConnected() ? "YES" : "NO");
    Serial.println();
}

void SerialDashboard::renderDiff(const PoolState& oldState, const PoolState& newState, const WifiManager& wifi, const MqttManager& mqtt)
{
    printHeader("STATE UPDATE");

    if (oldState.waterTemperature != newState.waterTemperature)
    {
        Serial.printf("Water temp      : %.1f -> %.1f °C\n", oldState.waterTemperature, newState.waterTemperature);
    }

    if (oldState.targetTemperature != newState.targetTemperature)
    {
        Serial.printf("Target temp     : %.1f -> %.1f °C\n", oldState.targetTemperature, newState.targetTemperature);
    }

    if (oldState.filterPump != newState.filterPump)
    {
        Serial.printf("Filter pump     : %s -> %s\n",
            oldState.filterPump ? "ON" : "OFF",
            newState.filterPump ? "ON" : "OFF");
    }

    if (oldState.heatingPump != newState.heatingPump)
    {
        Serial.printf("Heating pump    : %s -> %s\n",
            oldState.heatingPump ? "ON" : "OFF",
            newState.heatingPump ? "ON" : "OFF");
    }

    if (oldState.heatingAllowed != newState.heatingAllowed)
    {
        Serial.printf("Heating allowed : %s -> %s\n",
            oldState.heatingAllowed ? "YES" : "NO",
            newState.heatingAllowed ? "YES" : "NO");
    }

    if (oldState.mode != newState.mode)
    {
        Serial.printf("Mode            : %s -> %s\n",
            modeToString(oldState.mode),
            modeToString(newState.mode));
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
    switch (mode)
    {
        case PoolMode::Off:     return "Off";
        case PoolMode::Auto:    return "Auto";
        case PoolMode::Manual:  return "Manual";
        case PoolMode::Heating: return "Heating";
        default:                return "Unknown";
    }
}

void SerialDashboard::printHeader(const char* title) const
{
    Serial.println("========================================");
    Serial.println(title);
    Serial.println("========================================");
}