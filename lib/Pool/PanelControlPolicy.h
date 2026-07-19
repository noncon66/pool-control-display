#pragma once

#include <math.h>
#include <stdint.h>

#include "PoolState.h"

// Zentrale Bedienregeln des Panels. Sie steuern nur, welche Bedienelemente
// angeboten und welche MQTT-Wünsche gesendet werden. Loxone bleibt für die
// tatsächliche Freigabe und alle Sicherheitsgrenzen verantwortlich.
namespace PanelControlPolicy
{
    constexpr float MIN_TARGET_TEMPERATURE = 20.0f;
    constexpr float MAX_TARGET_TEMPERATURE = 32.0f;
    constexpr float TARGET_TEMPERATURE_STEP = 0.5f;

    inline bool isValidTargetTemperature(float value)
    {
        if (!isfinite(value) ||
            value < MIN_TARGET_TEMPERATURE ||
            value > MAX_TARGET_TEMPERATURE)
        {
            return false;
        }

        // Prüfen, ob der Wert ausgehend von 20 °C auf einem 0,5-°C-Schritt
        // liegt. Die Toleranz berücksichtigt typische Fließkomma-Rundungen.
        const float steps =
            (value - MIN_TARGET_TEMPERATURE) / TARGET_TEMPERATURE_STEP;
        return fabsf(steps - roundf(steps)) < 0.001f;
    }

    inline float adjustedTargetTemperature(float currentValue, int direction)
    {
        const float steps =
            (currentValue - MIN_TARGET_TEMPERATURE) / TARGET_TEMPERATURE_STEP;
        const float targetStep = direction < 0
            ? ceilf(steps - 0.001f) - 1.0f
            : floorf(steps + 0.001f) + 1.0f;
        return MIN_TARGET_TEMPERATURE +
               targetStep * TARGET_TEMPERATURE_STEP;
    }

    inline bool canAdjustTargetTemperature(const PoolState& state, uint32_t now)
    {
        (void)now;
        return state.hasMode &&
               state.mode == PoolMode::Auto &&
               state.hasTargetTemperature;
    }

    inline bool canSelectMode(const PoolState& state, bool mqttConnected, uint32_t now)
    {
        (void)now;
        return mqttConnected && state.hasMode;
    }

    inline bool canControlFilterPump(const PoolState& state, bool mqttConnected, uint32_t now)
    {
        (void)now;
        return mqttConnected &&
               state.isManualModeConfirmed() &&
               state.hasFilterPump;
    }

    inline bool canAdjustTargetTemperature(
        const PoolState& state,
        bool mqttConnected,
        uint32_t now)
    {
        return mqttConnected && canAdjustTargetTemperature(state, now);
    }
}
