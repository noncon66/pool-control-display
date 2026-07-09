#pragma once

#include <Arduino.h>

class WifiManager
{
public:
    // Startet den Stationsmodus und den ersten Verbindungsversuch.
    void begin();

    // Muss regelmäßig aufgerufen werden, damit Neuverbindungen möglich sind.
    void loop();

    // Schreibgeschützte Informationen für Anwendung und Dashboard.
    bool isConnected() const;
    const char* ipAddress() const;

private:
    // Begrenzt Verbindungsversuche auf einen Versuch alle fünf Sekunden.
    uint32_t _lastReconnectAttempt = 0;

    // Startet asynchron eine Verbindung und wartet nicht auf deren Erfolg.
    void connect();
};
