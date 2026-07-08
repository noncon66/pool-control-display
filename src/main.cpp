#include <Arduino.h>
#include "PoolState.h"

PoolState g_poolState;

void printPoolState()
{
    Serial.println("=== Pool Control Display ===");
    Serial.printf("Water temperature : %.1f °C\n", g_poolState.waterTemperature);
    Serial.printf("Target temperature: %.1f °C\n", g_poolState.targetTemperature);
    Serial.printf("Filter pump       : %s\n", g_poolState.filterPump ? "ON" : "OFF");
    Serial.printf("Heating pump      : %s\n", g_poolState.heatingPump ? "ON" : "OFF");
    Serial.printf("Heating allowed   : %s\n", g_poolState.heatingAllowed ? "YES" : "NO");
    Serial.printf("WiFi connected    : %s\n", g_poolState.wifiConnected ? "YES" : "NO");
    Serial.printf("MQTT connected    : %s\n", g_poolState.mqttConnected ? "YES" : "NO");
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    g_poolState.waterTemperature = 27.4f;
    g_poolState.targetTemperature = 29.0f;
    g_poolState.mode = PoolMode::Auto;

    printPoolState();
}

void loop()
{
    static uint32_t lastPrint = 0;

    if (millis() - lastPrint > 5000)
    {
        lastPrint = millis();
        printPoolState();
    }

    delay(10);
}