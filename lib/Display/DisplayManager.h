#pragma once

#include <Arduino.h>
#include <lvgl.h>

// Hardware integration for the Waveshare 4B panel, including LVGL display
// output and GT911 pointer input.
class DisplayManager
{
public:
    bool begin();
    void loop();

    bool isInitialized() const { return _displayReady; }
    bool isTouchReady() const { return _touchReady; }
    bool isLvglReady() const { return _lvglReady; }
    bool isLvglTouchReady() const { return _lvglTouchReady; }
    bool consumeTouchPress();
    void suppressCurrentTouchForLvgl();
    uint16_t width() const { return 480; }
    uint16_t height() const { return 480; }

    void setBacklightPercent(uint8_t percent);

private:
    bool _displayReady = false;
    bool _lvglReady = false;
    bool _lvglTouchReady = false;
    bool _backlightReady = false;
    bool _backlightPwmAttached = false;
    bool _screenBlanked = false;
    bool _touchReady = false;
    uint8_t _touchAddress = 0;
    bool _touchActive = false;
    bool _touchPressed = false;
    bool _suppressLvglTouchUntilRelease = false;
    uint16_t _touchX = 0;
    uint16_t _touchY = 0;
    uint32_t _lastTouchEvent = 0;

    bool initDisplayHardware();
    bool initTouchHardware();
    bool initLvglDisplay();
    bool initLvglTouch();
    bool ensureBacklightPwm();
    void drawDiagnosticScreen();
    bool ping(uint8_t address);
    bool readTouch(uint16_t reg, uint8_t* data, size_t length);
    bool writeTouch(uint16_t reg, uint8_t value);
    static void readLvglTouch(lv_indev_drv_t* driver, lv_indev_data_t* data);
};
