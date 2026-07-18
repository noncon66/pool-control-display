#pragma once

#include <Arduino.h>

// Hardware-only integration stage for the Waveshare 4B panel. LVGL and GUI
// event forwarding remain disabled until display, touch, and backlight have
// been verified together with the normal firmware.
class DisplayManager
{
public:
    bool begin();
    void loop();

    bool isInitialized() const { return _displayReady; }
    bool isTouchReady() const { return _touchReady; }
    bool consumeTouchPress();
    uint16_t width() const { return 480; }
    uint16_t height() const { return 480; }

    void setBacklightPercent(uint8_t percent);

private:
    bool _displayReady = false;
    bool _backlightReady = false;
    bool _backlightPwmAttached = false;
    bool _screenBlanked = false;
    bool _touchReady = false;
    uint8_t _touchAddress = 0;
    bool _touchActive = false;
    bool _touchPressed = false;
    uint32_t _lastTouchEvent = 0;

    bool initDisplayHardware();
    bool initTouchHardware();
    bool ensureBacklightPwm();
    void drawDiagnosticScreen();
    bool ping(uint8_t address);
    bool readTouch(uint16_t reg, uint8_t* data, size_t length);
    bool writeTouch(uint16_t reg, uint8_t value);
};
