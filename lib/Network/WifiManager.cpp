#include "WifiManager.h"

#include <WiFi.h>
#include "config.h"

void WifiManager::begin()
{
    WiFi.mode(WIFI_STA);
    connect();
}

void WifiManager::loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    const uint32_t now = millis();
    if (now - _lastReconnectAttempt >= 5000)
    {
        _lastReconnectAttempt = now;
        Serial.println("[WiFi] reconnecting...");
        connect();
    }
}

bool WifiManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

const char* WifiManager::ipAddress() const
{
    static String ip;
    ip = WiFi.localIP().toString();
    return ip.c_str();
}

void WifiManager::connect()
{
    Serial.printf("[WiFi] connecting to SSID '%s'\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}