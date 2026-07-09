#pragma once

#include "PoolState.h"

class WifiManager;
class MqttManager;

class SerialDashboard
{
public:
    // Das serielle Dashboard ist eine vorläufige, hardwareunabhängige Ansicht.
    // So sind WLAN, MQTT und Loxone bereits ohne Display beobachtbar.
    void begin();

    // renderFull() gibt alle Werte aus. renderDiff() zeigt nur Felder, die sich
    // seit der vorherigen PoolState-Kopie geändert haben.
    void renderFull(const PoolState& state, const WifiManager& wifi, const MqttManager& mqtt);
    void renderDiff(const PoolState& oldState, const PoolState& newState, const WifiManager& wifi, const MqttManager& mqtt);

private:
    // Wandelt das streng typisierte Enum in lesbaren Diagnosetext um.
    const char* modeToString(PoolMode mode) const;
    void printHeader(const char* title) const;
};
