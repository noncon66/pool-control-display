#pragma once

#include <math.h>
#include <stdint.h>

#include "PoolState.h"

// Beschreibt ausschließlich den Kommunikationsstand eines Bedienwunsches.
// Der tatsächliche Anlagenzustand steht weiterhin nur in PoolState.
enum class CommandProgress : uint8_t
{
    Idle,
    Pending,
    Confirmed,
    TimedOut
};

struct ModeCommandState
{
    CommandProgress progress = CommandProgress::Idle;
    PoolMode requestedValue = PoolMode::Off;
    uint32_t changedAt = 0;
};

struct TargetTemperatureCommandState
{
    CommandProgress progress = CommandProgress::Idle;
    float requestedValue = 0.0f;
    uint32_t changedAt = 0;
};

struct FilterPumpCommandState
{
    CommandProgress progress = CommandProgress::Idle;
    bool requestedValue = false;
    uint32_t changedAt = 0;
};

class PanelCommandState
{
public:
    // Nach fünf Sekunden ohne passende Loxone-Statusmeldung gilt ein Wunsch
    // als nicht bestätigt. Loxone könnte ihn dennoch bewusst abgelehnt haben.
    static constexpr uint32_t CONFIRMATION_TIMEOUT_MS = 5000;

    ModeCommandState mode;
    TargetTemperatureCommandState targetTemperature;
    FilterPumpCommandState filterPump;

    void markModePending(PoolMode value, uint32_t now)
    {
        mode.progress = CommandProgress::Pending;
        mode.requestedValue = value;
        mode.changedAt = now;
    }

    void markTargetTemperaturePending(float value, uint32_t now)
    {
        targetTemperature.progress = CommandProgress::Pending;
        targetTemperature.requestedValue = value;
        targetTemperature.changedAt = now;
    }

    void markFilterPumpPending(bool value, uint32_t now)
    {
        filterPump.progress = CommandProgress::Pending;
        filterPump.requestedValue = value;
        filterPump.changedAt = now;
    }

    void confirmMode(PoolMode confirmedValue, uint32_t now)
    {
        if (mode.progress == CommandProgress::Pending &&
            mode.requestedValue == confirmedValue)
        {
            mode.progress = CommandProgress::Confirmed;
            mode.changedAt = now;
        }
    }

    void confirmTargetTemperature(float confirmedValue, uint32_t now)
    {
        // MQTT überträgt die Solltemperatur mit einer Nachkommastelle. Eine
        // kleine Toleranz verhindert Probleme durch Fließkomma-Rundungen.
        if (targetTemperature.progress == CommandProgress::Pending &&
            fabsf(targetTemperature.requestedValue - confirmedValue) < 0.05f)
        {
            targetTemperature.progress = CommandProgress::Confirmed;
            targetTemperature.changedAt = now;
        }
    }

    void confirmFilterPump(bool confirmedValue, uint32_t now)
    {
        if (filterPump.progress == CommandProgress::Pending &&
            filterPump.requestedValue == confirmedValue)
        {
            filterPump.progress = CommandProgress::Confirmed;
            filterPump.changedAt = now;
        }
    }

    void updateTimeouts(uint32_t now)
    {
        updateTimeout(mode.progress, mode.changedAt, now);
        updateTimeout(targetTemperature.progress, targetTemperature.changedAt, now);
        updateTimeout(filterPump.progress, filterPump.changedAt, now);
    }

private:
    static void updateTimeout(CommandProgress& progress, uint32_t& changedAt, uint32_t now)
    {
        if (progress == CommandProgress::Pending &&
            now - changedAt >= CONFIRMATION_TIMEOUT_MS)
        {
            progress = CommandProgress::TimedOut;
            changedAt = now;
        }
    }
};
