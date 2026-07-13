# Pool Control Display

[![CI](https://github.com/noncon66/pool-control-display/actions/workflows/ci.yml/badge.svg)](https://github.com/noncon66/pool-control-display/actions/workflows/ci.yml)

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

OTA updates are disabled by default. To enable them later, set `OTA_ENABLED`
to `true` and configure a strong `OTA_PASSWORD` in the private
`PoolConfig.h`. OTA refuses to start with an empty password.

## Planned features

- LVGL-based touch UI (widget structure implemented; hardware activation pending)
- Wi-Fi + MQTT reconnect handling
- OTA updates (implemented, disabled by default)
- Pool dashboard
- Maintenance screen
- Settings screen
- Optional statistics/history view

## Hardware

Target device: Waveshare ESP32-S3-Touch-LCD-4B / ESP32-S3 Smart 86 Box,
4" wall touch panel, 480x480.

## User interface

The approved Loxone-inspired dashboard concept and interaction rules are
documented in [`docs/ui.md`](docs/ui.md).

## MQTT simulation

The dependency-free Python 3 simulator is recommended for macOS and Linux.
A PowerShell alternative is available for Windows and systems with PowerShell
7. Both publish Loxone status values and validate panel commands against the
documented rules. Usage is described in
[`docs/simulator.md`](docs/simulator.md).

## Loxone integration

The LoxBerry MQTT Gateway setup, virtual Loxone inputs and outputs, command
validation, retained status publishing, and commissioning checklist are
documented in [`docs/loxone.md`](docs/loxone.md).

## Project status

Hardware-independent application core in development:

- Wi-Fi and MQTT reconnect handling
- validated Loxone status model with unknown/stale data handling
- command validation and confirmation tracking
- serial diagnostic dashboard
- native state-model tests prepared

Display and touch support remain disabled until the delivered Waveshare panel
has been checked against the vendor demo and its ST7701 / GT911 driver setup
has been ported and tested on real hardware.

Automatic firmware builds, native tests, and simulator checks run through
GitHub Actions. See [`docs/ci.md`](docs/ci.md).
