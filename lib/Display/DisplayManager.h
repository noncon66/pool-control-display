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
    uint16_t width() const { return 480; }
    uint16_t height() const { return 480; }

    void setBacklightPercent(uint8_t percent);

private:
    bool _displayReady = false;
    bool _touchReady = false;
    uint8_t _touchAddress = 0;
    bool _touchActive = false;
    uint32_t _lastTouchEvent = 0;

    bool initDisplayHardware();
    bool initTouchHardware();
    bool ping(uint8_t address);
    bool readTouch(uint16_t reg, uint8_t* data, size_t length);
    bool writeTouch(uint16_t reg, uint8_t value);
};
