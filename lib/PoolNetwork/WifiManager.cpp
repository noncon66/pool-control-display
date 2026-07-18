#include "WifiManager.h"

#include <WiFi.h>
#include "PoolConfig.h"

void WifiManager::begin()
{
    // WIFI_STA verbindet den ESP32 mit einem vorhandenen WLAN. Der ESP32
    // eröffnet dabei keinen eigenen Access Point.
    WiFi.mode(WIFI_STA);
    connect();
}

void WifiManager::loop()
{
    // Solange die Verbindung besteht, ist nichts weiter zu tun.
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    const uint32_t now = millis();
    // Das Zeitintervall verhindert bei nicht erreichbarem Router dauerhaft
    // wiederholte Anfragen und übermäßige serielle Meldungen.
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
    // Der statische String bleibt nach dem Funktionsende bestehen. Dadurch
    // bleibt der von c_str() gelieferte Zeiger bis zum nächsten Aufruf gültig.
    static String ip;
    ip = WiFi.localIP().toString();
    return ip.c_str();
}

void WifiManager::connect()
{
    // Zugangsdaten stehen in der lokalen, von Git ignorierten PoolConfig.h.
    Serial.printf("[WiFi] connecting to SSID '%s'\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}
