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
fields distinguish confirmed values from initial storage defaults.

## Hardware separation

Display and touch libraries are disabled in `platformio.ini` until the exact
panel revision is known. Wi-Fi, MQTT and state handling can be developed and
tested independently.
