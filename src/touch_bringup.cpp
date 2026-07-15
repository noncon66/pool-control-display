#include <Arduino.h>
#include <Wire.h>

// Isolated GT911 diagnostic for Waveshare ESP32-S3-Touch-LCD-4B.
// Register layout, I2C pins, and reset ordering follow Waveshare's official
// V1.0 BSP. This source intentionally does not initialize the RGB display,
// LVGL, Wi-Fi, MQTT, or pool controls.
namespace
{
    constexpr uint8_t I2C_SDA = 47;
    constexpr uint8_t I2C_SCL = 48;
    constexpr uint8_t TCA9554_ADDRESS = 0x20;
    constexpr uint8_t GT911_PRIMARY_ADDRESS = 0x5D;
    constexpr uint8_t GT911_BACKUP_ADDRESS = 0x14;

    constexpr uint16_t GT911_CONFIG = 0x8047;
    constexpr uint16_t GT911_PRODUCT_ID = 0x8140;
    constexpr uint16_t GT911_STATUS = 0x814E;
    constexpr uint16_t GT911_FIRST_POINT = 0x814F;

    constexpr uint16_t DISPLAY_SIZE = 480;
    constexpr uint16_t CORNER_LIMIT = 96;
    constexpr uint16_t CORNER_HIGH = DISPLAY_SIZE - CORNER_LIMIT;

    uint8_t gt911Address = 0;
    bool touchActive = false;
    uint32_t lastTouchEvent = 0;
    uint8_t expectedCorner = 0;
    uint16_t minX = UINT16_MAX;
    uint16_t minY = UINT16_MAX;
    uint16_t maxX = 0;
    uint16_t maxY = 0;
    uint32_t lastHeartbeat = 0;

    const char *const cornerSequence[] = {"top-left", "top-right", "bottom-right", "bottom-left"};

    bool ping(uint8_t address)
    {
        Wire.beginTransmission(address);
        return Wire.endTransmission() == 0;
    }

    bool readRegister8(uint8_t address, uint8_t reg, uint8_t &value)
    {
        Wire.beginTransmission(address);
        Wire.write(reg);
        if (Wire.endTransmission(false) != 0 || Wire.requestFrom(address, static_cast<uint8_t>(1)) != 1)
        {
            return false;
        }
        value = Wire.read();
        return true;
    }

    bool writeRegister8(uint8_t address, uint8_t reg, uint8_t value)
    {
        Wire.beginTransmission(address);
        Wire.write(reg);
        Wire.write(value);
        return Wire.endTransmission() == 0;
    }

    bool readGt911(uint16_t reg, uint8_t *data, size_t length)
    {
        Wire.beginTransmission(gt911Address);
        Wire.write(static_cast<uint8_t>(reg >> 8));
        Wire.write(static_cast<uint8_t>(reg));
        if (Wire.endTransmission(false) != 0)
        {
            return false;
        }

        const size_t received = Wire.requestFrom(gt911Address, length);
        if (received != length)
        {
            return false;
        }
        for (size_t i = 0; i < length; ++i)
        {
            data[i] = Wire.read();
        }
        return true;
    }

    bool writeGt911(uint16_t reg, uint8_t value)
    {
        Wire.beginTransmission(gt911Address);
        Wire.write(static_cast<uint8_t>(reg >> 8));
        Wire.write(static_cast<uint8_t>(reg));
        Wire.write(value);
        return Wire.endTransmission() == 0;
    }

    bool resetPanelAndTouch()
    {
        uint8_t output = 0;
        uint8_t configuration = 0;
        if (!readRegister8(TCA9554_ADDRESS, 1, output) ||
            !readRegister8(TCA9554_ADDRESS, 3, configuration))
        {
            return false;
        }

        // Pins 5 and 6 are driven during Waveshare's shared panel/touch reset.
        configuration &= static_cast<uint8_t>(~(bit(5) | bit(6)));
        if (!writeRegister8(TCA9554_ADDRESS, 3, configuration))
        {
            return false;
        }

        output &= static_cast<uint8_t>(~bit(6));
        if (!writeRegister8(TCA9554_ADDRESS, 1, output))
        {
            return false;
        }
        delay(200);

        output &= static_cast<uint8_t>(~bit(5));
        if (!writeRegister8(TCA9554_ADDRESS, 1, output))
        {
            return false;
        }
        delay(200);

        output |= bit(5);
        if (!writeRegister8(TCA9554_ADDRESS, 1, output))
        {
            return false;
        }
        delay(200);

        // Waveshare releases expander pin 6 after the address/reset phase.
        configuration |= bit(6);
        return writeRegister8(TCA9554_ADDRESS, 3, configuration);
    }

    void scanI2c()
    {
        Serial.println("[Touch] I2C scan:");
        for (uint8_t address = 1; address < 0x7F; ++address)
        {
            if (ping(address))
            {
                Serial.printf("[Touch]   device 0x%02X\n", address);
            }
        }
    }

    const char *cornerFor(uint16_t x, uint16_t y)
    {
        if (x < CORNER_LIMIT && y < CORNER_LIMIT)
            return "top-left";
        if (x >= CORNER_HIGH && y < CORNER_LIMIT)
            return "top-right";
        if (x >= CORNER_HIGH && y >= CORNER_HIGH)
            return "bottom-right";
        if (x < CORNER_LIMIT && y >= CORNER_HIGH)
            return "bottom-left";
        return "none";
    }

    void reportPoint(const uint8_t *point)
    {
        const uint8_t trackId = point[0];
        const uint16_t x = static_cast<uint16_t>(point[1] | (point[2] << 8));
        const uint16_t y = static_cast<uint16_t>(point[3] | (point[4] << 8));
        const uint16_t strength = static_cast<uint16_t>(point[5] | (point[6] << 8));
        const char *corner = cornerFor(x, y);

        minX = min(minX, x);
        minY = min(minY, y);
        maxX = max(maxX, x);
        maxY = max(maxY, y);

        Serial.printf(
            "[Touch] raw x=%u y=%u strength=%u track=%u corner=%s range=[%u..%u,%u..%u]\n",
            x, y, strength, trackId, corner, minX, maxX, minY, maxY);

        if (expectedCorner < 4 && strcmp(corner, cornerSequence[expectedCorner]) == 0)
        {
            ++expectedCorner;
            if (expectedCorner == 4)
            {
                Serial.println("[Touch] corner sequence PASS; orientation is not swapped or mirrored");
            }
            else
            {
                Serial.printf("[Touch] next corner: %s\n", cornerSequence[expectedCorner]);
            }
        }
        else if (expectedCorner < 4)
        {
            Serial.printf("[Touch] expected corner: %s\n", cornerSequence[expectedCorner]);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("[Touch] Waveshare ESP32-S3-Touch-LCD-4B GT911 bring-up");

    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(400000);
    scanI2c();

    if (!ping(TCA9554_ADDRESS) || !resetPanelAndTouch())
    {
        Serial.println("[Touch] ERROR: TCA9554 reset sequence failed");
        return;
    }

    delay(100);
    gt911Address = ping(GT911_PRIMARY_ADDRESS) ? GT911_PRIMARY_ADDRESS :
                   ping(GT911_BACKUP_ADDRESS)  ? GT911_BACKUP_ADDRESS : 0;
    if (gt911Address == 0)
    {
        Serial.println("[Touch] ERROR: GT911 not found at 0x5D or 0x14");
        return;
    }

    uint8_t productId[3] = {};
    uint8_t config[5] = {};
    if (!readGt911(GT911_PRODUCT_ID, productId, sizeof(productId)) ||
        !readGt911(GT911_CONFIG, config, sizeof(config)))
    {
        Serial.println("[Touch] ERROR: GT911 identification read failed");
        gt911Address = 0;
        return;
    }

    const uint16_t configuredX = static_cast<uint16_t>(config[1] | (config[2] << 8));
    const uint16_t configuredY = static_cast<uint16_t>(config[3] | (config[4] << 8));
    Serial.printf("[Touch] GT911 found at 0x%02X, product=%c%c%c, config=%u, resolution=%ux%u\n",
                  gt911Address, productId[0], productId[1], productId[2],
                  config[0], configuredX, configuredY);
    Serial.println("[Touch] touch corners in order: top-left, top-right, bottom-right, bottom-left");
}

void loop()
{
    if (gt911Address == 0)
    {
        delay(1000);
        return;
    }

    uint8_t status = 0;
    if (!readGt911(GT911_STATUS, &status, 1))
    {
        Serial.println("[Touch] ERROR: status read failed");
        delay(100);
        return;
    }

    if ((status & 0x80) != 0)
    {
        const uint8_t points = status & 0x0F;
        if (points == 0)
        {
            touchActive = false;
        }
        else if (points <= 5)
        {
            uint8_t data[40] = {};
            if (readGt911(GT911_FIRST_POINT, data, points * 8))
            {
                lastTouchEvent = millis();
                if (!touchActive)
                {
                    reportPoint(data);
                }
                touchActive = true;
            }
        }
        writeGt911(GT911_STATUS, 0);
    }

    // Some GT911 firmware revisions do not always publish a separate release
    // frame. Treat a short period without coordinate updates as finger-up.
    if (touchActive && millis() - lastTouchEvent >= 150)
    {
        touchActive = false;
    }

    if (millis() - lastHeartbeat >= 5000)
    {
        lastHeartbeat = millis();
        if (expectedCorner < 4)
        {
            Serial.printf("[Touch] ready; waiting for %s\n", cornerSequence[expectedCorner]);
        }
    }
    delay(10);
}
