#pragma once

struct PoolData
{
    float temperature = 0;
    float targetTemperature = 28;

    bool filterPump = false;
    bool heatingPump = false;
    bool heatingAllowed = false;

    uint8_t mode = 0;
};