# Architecture

## Responsibility boundary

Loxone is the only controller of the pool installation. It owns all operating
logic, interlocks, limits and safety decisions.

The display is deliberately a thin MQTT client:

- It renders status values received from Loxone.
- It publishes user requests as MQTT commands.
- It never derives or changes plant state locally.
- A command does not update the displayed state optimistically. The display
  waits for the corresponding status message from Loxone.
- Missing values are shown as unknown, not replaced with plausible defaults.
- When MQTT is offline, command controls must be disabled.

`PoolState` is a cache of the latest confirmed Loxone status. Its `has...`
fields distinguish confirmed values from initial storage defaults. Update
timestamps remain available for diagnostics, but retained values remain usable
while MQTT is connected and do not require cyclic Loxone publishes.

`PoolStatusUpdater` maps validated MQTT status topics to `PoolState` and
confirms matching pending commands. It has no Arduino, Wi-Fi, or broker
dependency, so the complete topic-to-state contract can be tested natively.
Unknown topics and invalid payloads leave the confirmed state and its freshness
timestamps unchanged.

`PanelCommandState` tracks whether user requests are pending, confirmed, or
timed out without changing `PoolState`.

`PanelViewModel` combines connection state, known-value state, control policy,
and command progress into simple flags for the GUI. LVGL widgets use this model
instead of implementing their own permission rules.

`ScreenPowerPolicy` tracks the screen power state independently from pool
status and commands. It supports optional dimming and decides whether a touch
should be forwarded to the GUI. Dimming is disabled for the Waveshare 4B after
hardware tests showed visible PWM flicker; it remains fully lit until the
five-minute black/off transition. A touch from off only wakes the screen, so an
accidental first tap cannot trigger a pool command.

`OtaManager` handles optional password-protected firmware updates. OTA is
disabled by default and starts only after Wi-Fi is connected and a non-empty
password is configured.

## Hardware integration

The target panel is the Waveshare ESP32-S3-Touch-LCD-4B. Its ST7701 display,
GT911 touch controller, backlight, LVGL output/input drivers and safe wake-touch
handling are integrated in the normal `esp32-s3-panel` firmware target. The
isolated display and touch environments remain available for hardware
diagnostics.

Pool state, MQTT command policy and screen-power policy remain separated from
the hardware drivers and can still be developed and tested independently.
