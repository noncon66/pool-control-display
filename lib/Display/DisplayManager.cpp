#include "DisplayManager.h"

#include <Arduino_GFX_Library.h>
#include <Wire.h>

namespace
{
    constexpr uint16_t SCREEN_WIDTH = 480;
    constexpr uint16_t SCREEN_HEIGHT = 480;

    constexpr int I2C_SDA = 47;
    constexpr int I2C_SCL = 48;
    constexpr int LCD_BACKLIGHT = 4;
    constexpr uint8_t BACKLIGHT_RESOLUTION = 8;
    constexpr uint32_t BACKLIGHT_FREQUENCY = 20000;

    constexpr uint8_t GT911_PRIMARY_ADDRESS = 0x5D;
    constexpr uint8_t GT911_BACKUP_ADDRESS = 0x14;
    constexpr uint16_t GT911_CONFIG = 0x8047;
    constexpr uint16_t GT911_PRODUCT_ID = 0x8140;
    constexpr uint16_t GT911_STATUS = 0x814E;
    constexpr uint16_t GT911_FIRST_POINT = 0x814F;

    Arduino_XCA9554SWSPI expander(
        7,
        0,
        2,
        1,
        &Wire,
        0x20);

    Arduino_ESP32RGBPanel rgbPanel(
        17 /* DE */, 3 /* VSYNC */, 46 /* HSYNC */, 9 /* PCLK */,
        10 /* B0 */, 11 /* B1 */, 12 /* B2 */, 13 /* B3 */, 14 /* B4 */,
        21 /* G0 */, 8 /* G1 */, 18 /* G2 */, 45 /* G3 */, 38 /* G4 */, 39 /* G5 */,
        40 /* R0 */, 41 /* R1 */, 42 /* R2 */, 2 /* R3 */, 1 /* R4 */,
        1 /* HSYNC polarity */, 10 /* front porch */, 8 /* pulse */, 50 /* back porch */,
        1 /* VSYNC polarity */, 10 /* front porch */, 8 /* pulse */, 20 /* back porch */);

    Arduino_RGB_Display display(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        &rgbPanel,
        0 /* rotation */,
        true /* auto flush */,
        &expander,
        GFX_NOT_DEFINED /* reset is handled through the expander */,
        st7701_type1_init_operations,
        sizeof(st7701_type1_init_operations));

    void resetPanelAndTouch()
    {
        expander.pinMode(5, OUTPUT);
        expander.pinMode(6, OUTPUT);
        expander.digitalWrite(6, LOW);
        delay(200);
        expander.digitalWrite(5, LOW);
        delay(200);
        expander.digitalWrite(5, HIGH);
        delay(200);
        expander.pinMode(6, INPUT);
    }
}

bool DisplayManager::begin()
{
    Serial.println("[Display] hardware integration begin");

    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(400000);

    pinMode(LCD_BACKLIGHT, OUTPUT);
    digitalWrite(LCD_BACKLIGHT, HIGH);
    _backlightReady = true;
    setBacklightPercent(0);

    if (!initDisplayHardware())
    {
        Serial.println("[Display] ERROR: ST7701 initialization failed");
        return false;
    }
    _displayReady = true;

    _touchReady = initTouchHardware();
    if (!_touchReady)
    {
        Serial.println("[Display] WARNING: GT911 unavailable; display remains active");
    }

    drawDiagnosticScreen();
    setBacklightPercent(100);

    Serial.printf("[Display] ready; touch=%s, LVGL=disabled\n", _touchReady ? "ready" : "failed");
    return true;
}

void DisplayManager::loop()
{
    if (!_touchReady)
    {
        return;
    }

    uint8_t status = 0;
    if (!readTouch(GT911_STATUS, &status, 1))
    {
        Serial.println("[Display] WARNING: GT911 status read failed");
        return;
    }

    if ((status & 0x80) != 0)
    {
        const uint8_t points = status & 0x0F;
        if (points == 0)
        {
            _touchActive = false;
        }
        else if (points <= 5)
        {
            uint8_t data[40] = {};
            if (readTouch(GT911_FIRST_POINT, data, points * 8))
            {
                _lastTouchEvent = millis();
                if (!_touchActive)
                {
                    const uint16_t x = static_cast<uint16_t>(data[1] | (data[2] << 8));
                    const uint16_t y = static_cast<uint16_t>(data[3] | (data[4] << 8));
                    const uint16_t strength = static_cast<uint16_t>(data[5] | (data[6] << 8));
                    Serial.printf("[Display] touch raw x=%u y=%u strength=%u\n", x, y, strength);
                    _touchPressed = true;
                }
                _touchActive = true;
            }
        }
        writeTouch(GT911_STATUS, 0);
    }

    if (_touchActive && millis() - _lastTouchEvent >= 150)
    {
        _touchActive = false;
    }
}

bool DisplayManager::consumeTouchPress()
{
    const bool pressed = _touchPressed;
    _touchPressed = false;
    return pressed;
}

void DisplayManager::setBacklightPercent(uint8_t percent)
{
    if (!_backlightReady)
    {
        return;
    }

    if (percent > 100)
    {
        percent = 100;
    }

    // Use unambiguous static levels at both endpoints. This mirrors the
    // verified Waveshare bring-up and avoids LEDC's special full-duty values.
    if (percent == 0 || percent == 100)
    {
        if (percent == 0 && _displayReady && !_screenBlanked)
        {
            display.fillScreen(BLACK);
            _screenBlanked = true;
            Serial.println("[Display] framebuffer blanked");
        }
        else if (percent == 100 && _displayReady && _screenBlanked)
        {
            // Restore the framebuffer while the backlight is still off so the
            // user never sees an incomplete redraw during wake-up.
            drawDiagnosticScreen();
        }

        if (_backlightPwmAttached)
        {
            ledcDetach(LCD_BACKLIGHT);
            _backlightPwmAttached = false;
            pinMode(LCD_BACKLIGHT, OUTPUT);
        }

        const uint8_t level = percent == 0 ? HIGH : LOW;
        digitalWrite(LCD_BACKLIGHT, level);
        Serial.printf(
            "[Display] backlight target=%u%% gpio=%s readback=%u\n",
            percent,
            level == HIGH ? "HIGH" : "LOW",
            digitalRead(LCD_BACKLIGHT));
        return;
    }

    if (!ensureBacklightPwm())
    {
        return;
    }

    const uint32_t maxDuty = (1UL << BACKLIGHT_RESOLUTION) - 1;
    const uint32_t duty = maxDuty * percent / 100;
    if (!ledcWrite(LCD_BACKLIGHT, duty))
    {
        Serial.println("[Display] WARNING: backlight PWM write failed");
        return;
    }

    // The hardware applies a new duty at the next PWM cycle. Waiting longer
    // than one 20 kHz cycle makes the diagnostic readback deterministic.
    delayMicroseconds(100);

    Serial.printf(
        "[Display] backlight target=%u%% duty=%lu readback=%lu\n",
        percent,
        static_cast<unsigned long>(duty),
        static_cast<unsigned long>(ledcRead(LCD_BACKLIGHT)));
}

bool DisplayManager::ensureBacklightPwm()
{
    if (_backlightPwmAttached)
    {
        return true;
    }

    if (!ledcAttach(LCD_BACKLIGHT, BACKLIGHT_FREQUENCY, BACKLIGHT_RESOLUTION))
    {
        Serial.println("[Display] ERROR: backlight PWM attach failed");
        return false;
    }
    if (!ledcOutputInvert(LCD_BACKLIGHT, true))
    {
        Serial.println("[Display] ERROR: backlight PWM inversion failed");
        ledcDetach(LCD_BACKLIGHT);
        return false;
    }

    _backlightPwmAttached = true;
    return true;
}

void DisplayManager::drawDiagnosticScreen()
{
    display.fillScreen(WHITE);
    display.setTextColor(RED);
    display.setTextSize(3);
    display.setCursor(24, 24);
    display.println("Pool Control");
    display.setTextColor(BLACK);
    display.setTextSize(2);
    display.println("Hardware integration OK");
    display.println(_touchReady ? "GT911 touch ready" : "GT911 touch FAILED");
    display.println("LVGL disabled");
    _screenBlanked = false;
}

bool DisplayManager::initDisplayHardware()
{
    Serial.println("[Display] reset TCA9554/ST7701/GT911");
    resetPanelAndTouch();

    if (!display.begin())
    {
        return false;
    }
    Serial.println("[Display] ST7701 initialized");
    return true;
}

bool DisplayManager::initTouchHardware()
{
    _touchAddress = ping(GT911_PRIMARY_ADDRESS) ? GT911_PRIMARY_ADDRESS :
                    ping(GT911_BACKUP_ADDRESS)  ? GT911_BACKUP_ADDRESS : 0;
    if (_touchAddress == 0)
    {
        return false;
    }

    uint8_t productId[3] = {};
    uint8_t config[5] = {};
    if (!readTouch(GT911_PRODUCT_ID, productId, sizeof(productId)) ||
        !readTouch(GT911_CONFIG, config, sizeof(config)))
    {
        _touchAddress = 0;
        return false;
    }

    const uint16_t configuredX = static_cast<uint16_t>(config[1] | (config[2] << 8));
    const uint16_t configuredY = static_cast<uint16_t>(config[3] | (config[4] << 8));
    Serial.printf(
        "[Display] GT911 address=0x%02X product=%c%c%c config=%u resolution=%ux%u\n",
        _touchAddress,
        productId[0], productId[1], productId[2], config[0], configuredX, configuredY);

    return configuredX == SCREEN_WIDTH && configuredY == SCREEN_HEIGHT;
}

bool DisplayManager::ping(uint8_t address)
{
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

bool DisplayManager::readTouch(uint16_t reg, uint8_t* data, size_t length)
{
    Wire.beginTransmission(_touchAddress);
    Wire.write(static_cast<uint8_t>(reg >> 8));
    Wire.write(static_cast<uint8_t>(reg));
    if (Wire.endTransmission(false) != 0)
    {
        return false;
    }

    if (Wire.requestFrom(_touchAddress, length) != length)
    {
        return false;
    }
    for (size_t i = 0; i < length; ++i)
    {
        data[i] = Wire.read();
    }
    return true;
}

bool DisplayManager::writeTouch(uint16_t reg, uint8_t value)
{
    Wire.beginTransmission(_touchAddress);
    Wire.write(static_cast<uint8_t>(reg >> 8));
    Wire.write(static_cast<uint8_t>(reg));
    Wire.write(value);
    return Wire.endTransmission() == 0;
}
