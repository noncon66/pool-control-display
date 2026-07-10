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

`PanelCommandState` tracks whether user requests are pending, confirmed, or
timed out without changing `PoolState`.

`PanelViewModel` combines connection state, data freshness, control policy, and
command progress into simple flags for the future GUI. LVGL widgets should use
this model instead of implementing their own permission rules.

`ScreenPowerPolicy` tracks the screen power state independently from pool
status and commands. It decides when the display is awake, dimmed, or off and
whether a touch should be forwarded to the GUI. A touch from dimmed or off only
wakes the screen, so an accidental first tap cannot trigger a pool command.

`OtaManager` handles optional password-protected firmware updates. OTA is
disabled by default and starts only after Wi-Fi is connected and a non-empty
password is configured.

## Hardware separation

Display and touch libraries are disabled in `platformio.ini` until the exact
panel revision is known. Wi-Fi, MQTT, screen-power policy, and state handling
can be developed and tested independently.
