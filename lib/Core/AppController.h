#pragma once

#include <Arduino.h>
#include "PoolState.h"
#include "WifiManager.h"
#include "MqttManager.h"
#include "SerialDashboard.h"

class AppController
{
public:
    AppController();

    void begin();
    void loop();

private:
    PoolState _state;
    PoolState _lastState;

    WifiManager _wifi;
    MqttManager _mqtt;
    SerialDashboard _dashboard;

    uint32_t _lastPeriodicDashboardUpdate = 0;

    void updateConnectivityState();
    void handleStateChanges();
    bool hasStateChanged() const;
};