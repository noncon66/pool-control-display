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
    float targetTemperature = 0.0f;

    bool filterPump = false;
    bool heatingPump = false;
    bool heatingAllowed = false;

    PoolMode mode = PoolMode::Off;

    // Values are only valid after Loxone has published them.
    bool hasWaterTemperature = false;
    bool hasTargetTemperature = false;
    bool hasFilterPump = false;
    bool hasHeatingPump = false;
    bool hasHeatingAllowed = false;
    bool hasMode = false;

    // millis() value of the most recent valid status message from Loxone.
    uint32_t lastStatusUpdateAt = 0;

    bool wifiConnected = false;
    bool mqttConnected = false;
};
