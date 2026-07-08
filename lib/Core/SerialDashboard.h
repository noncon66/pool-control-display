#pragma once

#include "PoolState.h"

class WifiManager;
class MqttManager;

class SerialDashboard
{
public:
    void begin();

    void renderFull(const PoolState& state, const WifiManager& wifi, const MqttManager& mqtt);
    void renderDiff(const PoolState& oldState, const PoolState& newState, const WifiManager& wifi, const MqttManager& mqtt);

private:
    const char* modeToString(PoolMode mode) const;
    void printHeader(const char* title) const;
};