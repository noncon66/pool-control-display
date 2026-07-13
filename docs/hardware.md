# Hardware status

The target device has been identified as the Waveshare ESP32-S3-Touch-LCD-4B,
sold by BerryBase as the Waveshare ESP32-S3 Smart 86 Box.

Known hardware facts from the vendor documentation:

- Product: Waveshare ESP32-S3-Touch-LCD-4B
- Shop link: https://www.berrybase.at/waveshare-esp32-s3-smart-86-box-4-zoll-ips-480x480-wifi-bluetooth5-touch-audio-ki-240mhz
- Vendor wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4B
- MCU module: ESP32-S3-WROOM-1-N16R8
- CPU: dual-core Xtensa LX7, up to 240 MHz
- Wireless: 2.4 GHz Wi-Fi and Bluetooth 5 / BLE
- Flash: 16 MB
- PSRAM: 8 MB
- Display: 4 inch capacitive touch LCD, 480 x 480, 65K color
- Display controller: ST7701
- Touch controller: GT911 over I2C
- Power management: AXP2101
- RTC: PCF85063
- IMU: QMI8658
- Audio: ES8311 codec, ES7210 microphone / echo-cancellation chip
- Expansion: GPIO/UART/USB plus a 2.0 mm expansion header
- Power: USB-C and optional 3.7 V PH2.0 lithium battery header

The current code in `lib/Display` is still an experimental placeholder and is
excluded from builds. The official Waveshare demo and schematic must be used as
the source of truth for display timing, ST7701 initialization, GT911 touch
setup, backlight control, and any IO-expander usage.

## Verified Waveshare V1.0 reference

The official Waveshare V1.0 BSP and Arduino-v3.2.0 demo have now been checked.
They define the following display connections:

- I2C: SDA GPIO 47, SCL GPIO 48
- RGB control: DE 17, VSYNC 3, HSYNC 46, PCLK 9
- RGB data: B0..B4 = 10, 11, 12, 13, 14
- RGB data: G0..G5 = 21, 8, 18, 45, 38, 39
- RGB data: R0..R4 = 40, 41, 42, 2, 1
- Backlight: GPIO 4, inverted PWM polarity
- TCA9554 address: `0x20`
- ST7701 control through TCA9554 software SPI: CS 0, SDA 1, SCL 2
- Panel reset sequence through TCA9554 pins 5 and 6
- GT911 and the TCA9554 share the board I2C bus

The vendor currently requires Arduino ESP32 core 3.2.0 or newer. The normal
firmware still uses PlatformIO's Arduino core 2.0.17, so a separate
`esp32-s3-display-bringup` environment uses the matching, pinned pioarduino
core 3.2.0. This
keeps the hardware experiment isolated until the complete application has been
validated with the newer core.

Build the isolated smoke test on Windows with:

```powershell
.\tools\build_display_bringup.ps1
```

For a common workflow on macOS, Windows, and Linux, use the Python launcher:

```bash
python3 tools/display_bringup.py build
python3 tools/display_bringup.py upload --port /dev/cu.usbmodem1101
python3 tools/display_bringup.py monitor --port /dev/cu.usbmodem1101
```

On Windows, use `python` instead of `python3` if that is how Python is
installed. The port can be omitted when only one compatible serial device is
connected. The PlatformIO extension for VS Code already includes PlatformIO
Core. For terminal-only use, follow PlatformIO's official installer guide:
https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html

To find the device name on macOS, connect the board and run:

```bash
pio device list
```

Both scripts use `.pio/bringup-core` as a separate PlatformIO core directory.
This prevents the pinned Arduino 3.2.0 packages from replacing the normal
firmware's Arduino 2.0.17 packages in the user's global PlatformIO cache.

It displays a white screen with red and black test text. It does not initialize
touch, LVGL, Wi-Fi, MQTT, or any pool controls.

Before enabling display support:

- download and archive the official Waveshare demo package (reviewed locally)
- record the exact PCB revision printed on the delivered device; the current
  bring-up environment is explicitly based on V1.0
- compare the delivered board with the vendor schematic
- port the official ST7701 and GT911 setup instead of relying on generic RGB
  panel pins
- identify backlight control and active level
- verify USB serial upload, boot mode, and monitor output
- confirm display orientation and touch coordinate mapping on the real panel
- run a minimal display fill test, a touch coordinate test, and then the LVGL
  main screen
