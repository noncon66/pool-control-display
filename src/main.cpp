#include <Arduino.h>

#include "PoolState.h"
#include "WifiManager.h"
#include "MqttManager.h"

PoolState g_poolState;
WifiManager g_wifi;
MqttManager g_mqtt(g_poolState);

static uint32_t g_lastPrint = 0;

static void printState()
{
    Serial.println("=== Pool State ===");
    Serial.printf("Water temp      : %.1f °C\n", g_poolState.waterTemperature);
    Serial.printf("Target temp     : %.1f °C\n", g_poolState.targetTemperature);
    Serial.printf("Filter pump     : %s\n", g_poolState.filterPump ? "ON" : "OFF");
    Serial.printf("Heating pump    : %s\n", g_poolState.heatingPump ? "ON" : "OFF");
    Serial.printf("Heating allowed : %s\n", g_poolState.heatingAllowed ? "YES" : "NO");
    Serial.printf("Mode            : %u\n", static_cast<uint8_t>(g_poolState.mode));
    Serial.printf("WiFi connected  : %s\n", g_wifi.isConnected() ? "YES" : "NO");
    Serial.printf("MQTT connected  : %s\n", g_mqtt.isConnected() ? "YES" : "NO");
    if (g_wifi.isConnected())
    {
        Serial.printf("IP address      : %s\n", g_wifi.ipAddress());
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("=== Pool Control Display ===");
    Serial.println("Starting core services...");

    g_wifi.begin();
    g_mqtt.begin(g_wifi);
}

void loop()
{
    g_wifi.loop();
    g_mqtt.loop();

    g_poolState.wifiConnected = g_wifi.isConnected();
    g_poolState.mqttConnected = g_mqtt.isConnected();

    if (millis() - g_lastPrint >= 5000)
    {
        g_lastPrint = millis();
        printState();
    }

    delay(10);
}