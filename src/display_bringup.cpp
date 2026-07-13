#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>

// Hardware smoke test for Waveshare ESP32-S3-Touch-LCD-4B V1.0.
// Pin mapping and reset sequence are taken from Waveshare's official
// Arduino-v3.2.0 01_HelloWorld demo. This source is built only by the
// esp32-s3-display-bringup environment.
namespace
{
    constexpr int I2C_SDA = 47;
    constexpr int I2C_SCL = 48;
    constexpr int LCD_BACKLIGHT = 4;

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
        480,
        480,
        &rgbPanel,
        0 /* rotation */,
        true /* auto flush */,
        &expander,
        GFX_NOT_DEFINED /* reset is handled through the expander */,
        st7701_type1_init_operations,
        sizeof(st7701_type1_init_operations));

    void resetPanel()
    {
        expander.pinMode(5, OUTPUT);
        expander.pinMode(6, OUTPUT);
        expander.digitalWrite(6, LOW);
        delay(200);
        expander.digitalWrite(5, LOW);
        delay(200);
        expander.digitalWrite(5, HIGH);
        delay(200);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("[Bring-up] Waveshare ESP32-S3-Touch-LCD-4B");

    Wire.begin(I2C_SDA, I2C_SCL);
    resetPanel();

    // The official BSP drives the GPIO 4 backlight with inverted polarity.
    // Keep it off while the RGB panel and framebuffer are initialized.
    pinMode(LCD_BACKLIGHT, OUTPUT);
    digitalWrite(LCD_BACKLIGHT, HIGH);

    if (!display.begin())
    {
        Serial.println("[Bring-up] display initialization failed");
        return;
    }

    display.fillScreen(WHITE);
    display.setTextColor(RED);
    display.setTextSize(3);
    display.setCursor(24, 24);
    display.println("Pool Control");
    display.setTextColor(BLACK);
    display.setTextSize(2);
    display.println("Display bring-up OK");
    digitalWrite(LCD_BACKLIGHT, LOW);
    Serial.println("[Bring-up] display initialized");
}

void loop()
{
    delay(1000);
}
