# Pool Control Display

ESP32-S3 based wall touch display for pool control via MQTT, designed to integrate with Loxone.

## Goals

- 4" wall-mounted touch panel based on ESP32-S3
- MQTT communication with Loxone / LoxBerry
- Touch UI for pool mode, target temperature and pump control
- Live display of water temperature, heating state and system status
- Clean modular architecture with PlatformIO

## Configuration

Create the private device configuration:

```sh
cp include/PoolConfig.example.h include/PoolConfig.h
```

Then edit `include/PoolConfig.h` with the Wi-Fi and MQTT settings for the
device. This file is ignored by Git; only the example file is committed.

## Planned features

- LVGL-based touch UI
- Wi-Fi + MQTT reconnect handling
- OTA updates
- Pool dashboard
- Maintenance screen
- Settings screen
- Optional statistics/history view

## Hardware

Target device: ESP32-S3 4" wall touch panel (480x480)

## Project status

Hardware-independent application core in development:

- Wi-Fi and MQTT reconnect handling
- validated Loxone status model with unknown/stale data handling
- command validation and confirmation tracking
- serial diagnostic dashboard
- native state-model tests prepared

Display and touch support remain disabled until the exact panel revision,
controller types and pin mapping are known.
