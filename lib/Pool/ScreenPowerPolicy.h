#pragma once

#include <stdint.h>

#ifndef SCREEN_POWER_DIM_AFTER_MS
#define SCREEN_POWER_DIM_AFTER_MS 60000UL
#endif

#ifndef SCREEN_POWER_OFF_AFTER_MS
#define SCREEN_POWER_OFF_AFTER_MS 300000UL
#endif

enum class ScreenPowerLevel : uint8_t
{
    Awake,
    Dimmed,
    Off
};

// Hardwareunabhaengige Regeln fuer Hintergrundbeleuchtung und Touch-Wakeup.
// Der spaetere Displaytreiber setzt nur noch die passende Helligkeit um.
class ScreenPowerPolicy
{
public:
    static constexpr uint32_t DIM_AFTER_MS = SCREEN_POWER_DIM_AFTER_MS;
    static constexpr uint32_t OFF_AFTER_MS = SCREEN_POWER_OFF_AFTER_MS;

    static constexpr uint8_t AWAKE_BRIGHTNESS_PERCENT = 100;
    static constexpr uint8_t DIMMED_BRIGHTNESS_PERCENT = 10;
    static constexpr uint8_t OFF_BRIGHTNESS_PERCENT = 0;

    explicit ScreenPowerPolicy(bool dimmingEnabled = true)
        : _dimmingEnabled(dimmingEnabled)
    {
    }

    void begin(uint32_t now)
    {
        _lastActivityAt = now;
        _level = ScreenPowerLevel::Awake;
    }

    ScreenPowerLevel update(uint32_t now)
    {
        const uint32_t inactiveFor = now - _lastActivityAt;
        if (inactiveFor >= OFF_AFTER_MS)
        {
            _level = ScreenPowerLevel::Off;
        }
        else if (_dimmingEnabled && inactiveFor >= DIM_AFTER_MS)
        {
            _level = ScreenPowerLevel::Dimmed;
        }
        else
        {
            _level = ScreenPowerLevel::Awake;
        }

        return _level;
    }

    // Gibt true zurueck, wenn der Touch als Bedienung weitergegeben werden
    // darf. Im gedimmten oder ausgeschalteten Zustand weckt der erste Touch nur.
    bool handleTouch(uint32_t now)
    {
        const bool canForwardTouch = _level == ScreenPowerLevel::Awake;
        _lastActivityAt = now;
        _level = ScreenPowerLevel::Awake;
        return canForwardTouch;
    }

    ScreenPowerLevel level() const
    {
        return _level;
    }

    uint8_t brightnessPercent() const
    {
        switch (_level)
        {
            case ScreenPowerLevel::Awake:
                return AWAKE_BRIGHTNESS_PERCENT;
            case ScreenPowerLevel::Dimmed:
                return DIMMED_BRIGHTNESS_PERCENT;
            case ScreenPowerLevel::Off:
                return OFF_BRIGHTNESS_PERCENT;
            default:
                return OFF_BRIGHTNESS_PERCENT;
        }
    }

private:
    bool _dimmingEnabled = true;
    uint32_t _lastActivityAt = 0;
    ScreenPowerLevel _level = ScreenPowerLevel::Awake;
};
