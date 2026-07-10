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

Before enabling display support:

- download and archive the official Waveshare demo package
- record the exact PCB revision printed on the delivered device
- compare the delivered board with the vendor schematic
- port the official ST7701 and GT911 setup instead of relying on generic RGB
  panel pins
- identify backlight control and active level
- verify USB serial upload, boot mode, and monitor output
- confirm display orientation and touch coordinate mapping on the real panel
- run a minimal display fill test, a touch coordinate test, and then the LVGL
  main screen
