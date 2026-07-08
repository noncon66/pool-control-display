#pragma once

#include <stdint.h>

enum class PoolMode : uint8_t
{
    Off = 0,
    Auto = 1,
    Manual = 2,
    Heating = 3
};

struct PoolState
{
    float waterTemperature = 0.0f;
    float targetTemperature = 28.0f;

    bool filterPump = false;
    bool heatingPump = false;
    bool heatingAllowed = false;

    PoolMode mode = PoolMode::Auto;

    bool wifiConnected = false;
    bool mqttConnected = false;
};