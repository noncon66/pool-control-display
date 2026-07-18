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
fields distinguish confirmed values from initial storage defaults. Values that
influence a control permission have their own update timestamp. A current
temperature message therefore cannot make an old operating mode appear current.

`PoolStatusUpdater` maps validated MQTT status topics to `PoolState` and
confirms matching pending commands. It has no Arduino, Wi-Fi, or broker
dependency, so the complete topic-to-state contract can be tested natively.
Unknown topics and invalid payloads leave the confirmed state and its freshness
timestamps unchanged.

`PanelCommandState` tracks whether user requests are pending, confirmed, or
timed out without changing `PoolState`.

`PanelViewModel` combines connection state, data freshness, control policy, and
command progress into simple flags for the future GUI. LVGL widgets should use
this model instead of implementing their own permission rules.

`ScreenPowerPolicy` tracks the screen power state independently from pool
status and commands. It supports optional dimming and decides whether a touch
should be forwarded to the GUI. Dimming is disabled for the Waveshare 4B after
hardware tests showed visible PWM flicker; it remains fully lit until the
five-minute black/off transition. A touch from off only wakes the screen, so an
accidental first tap cannot trigger a pool command.

`OtaManager` handles optional password-protected firmware updates. OTA is
disabled by default and starts only after Wi-Fi is connected and a non-empty
password is configured.

## Hardware separation

The target panel is the Waveshare ESP32-S3-Touch-LCD-4B. Display and touch
libraries are still disabled in `platformio.ini` until the delivered board has
been checked against the vendor demo and the ST7701 display, GT911 touch, and
backlight setup have been verified on real hardware.

Wi-Fi, MQTT, screen-power policy, and state handling can be developed and
tested independently.
