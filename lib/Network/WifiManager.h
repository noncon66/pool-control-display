#pragma once

#include <Arduino.h>

class WifiManager
{
public:
    void begin();
    void loop();

    bool isConnected() const;
    const char* ipAddress() const;

private:
    uint32_t _lastReconnectAttempt = 0;
    void connect();
};