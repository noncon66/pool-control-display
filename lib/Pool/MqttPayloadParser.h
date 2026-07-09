#pragma once

#include <math.h>
#include <stdlib.h>

#include "PoolState.h"

// Hilfsfunktionen zum strikten Auswerten der MQTT-Nutzdaten.
//
// Wichtig: Der übergebene Zielwert wird nur verändert, wenn die komplette
// Nachricht gültig ist. Eine fehlerhafte MQTT-Nachricht kann dadurch niemals
// einen bereits bestätigten Loxone-Wert überschreiben.
namespace MqttPayloadParser
{
    inline bool parseFloat(const char* payload, float& result)
    {
        if (!payload || *payload == '\0')
        {
            return false;
        }

        char* end = nullptr;
        const float parsedValue = strtof(payload, &end);

        if (end == payload || *end != '\0' || !isfinite(parsedValue))
        {
            return false;
        }

        result = parsedValue;
        return true;
    }

    inline bool parseMode(const char* payload, PoolMode& result)
    {
        if (!payload || payload[0] == '\0' || payload[1] != '\0')
        {
            return false;
        }

        switch (payload[0])
        {
            case '0':
                result = PoolMode::Off;
                return true;
            case '1':
                result = PoolMode::Auto;
                return true;
            case '2':
                result = PoolMode::Manual;
                return true;
            default:
                return false;
        }
    }
}
