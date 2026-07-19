# MQTT Topics

This document is the communication contract between Loxone and the display.
The display never treats a sent command as confirmed state. It waits until
Loxone publishes the corresponding status topic.

The implementation through the LoxBerry MQTT Gateway and virtual Loxone inputs
and outputs is documented in [`loxone.md`](loxone.md).

## Status topics (published by Loxone / consumed by the display)

| Topic | Payload | Meaning |
|---|---:|---|
| `pool/status/waterTemp` | `27.4` | Current pool water temperature |
| `pool/status/targetTemp` | `29.0` | Target temperature |
| `pool/status/filterPump` | `0` / `1` | Filter pump state |
| `pool/status/heatingPump` | `0` / `1` | Heating pump state |
| `pool/status/heatingAllowed` | `0` / `1` | Heating permission from heating system |
| `pool/status/isHeating` | `0` / `1` | Pool is currently being heated |
| `pool/status/mode` | `1..3` | Pool operating mode |

Status topics should be published as retained MQTT messages. This allows the
display to receive the latest confirmed Loxone values immediately after a
restart or reconnect.

Mode values:

| Value | Meaning |
|---:|---|
| `1` | Automatic |
| `2` | Manual |
| `3` | Off |

## Command topics (published by the display / consumed by Loxone)

| Topic | Payload | Meaning |
|---|---:|---|
| `pool/cmd/mode` | `1..3` | Set user-selectable pool mode |
| `pool/cmd/targetTemp` | float | Set target temperature |
| `pool/cmd/filterPump` | `0` / `1` | Manually switch filter pump |

Command topics must not be retained. They represent a user request at a
specific moment, not a persistent state. Loxone validates and processes the
request, then publishes the resulting confirmed value on the matching status
topic.

Heating is not an operating mode. Loxone publishes it independently on
`pool/status/isHeating`; the display only presents that information.

## Filter pump command

The display enables the filter pump control only when all of these conditions
are true:

- MQTT is connected.
- Loxone has confirmed operating mode `2` (Manual).
- A confirmed filter-pump status has been received.

This panel-side check is only user-interface guidance. Loxone remains
authoritative and must independently reject `pool/cmd/filterPump` whenever its
own current operating mode or safety conditions do not permit manual control.

## Target temperature command

The display offers target-temperature changes only when:

- MQTT is connected.
- Loxone has confirmed operating mode `1` (Automatic).
- A confirmed target temperature has been received.
- The requested value is between `20.0 °C` and `32.0 °C`.
- The requested value uses a `0.5 °C` step.

If Loxone confirms an intermediate value that is not on this grid, Plus and
Minus snap to the next valid half-degree value instead of propagating the
offset.

These are panel interaction rules. Loxone must independently validate every
target-temperature command before applying it.

## Command confirmation

After publishing a command, the display tracks it separately from `PoolState`:

- `Pending`: the command was published and the display is waiting for Loxone.
- `Confirmed`: Loxone published the requested value on the matching status
  topic.
- `TimedOut`: no matching status arrived within five seconds.

The display never copies the requested value into `PoolState`. A different
status value means that Loxone did not confirm the request. The UI shows
“Wird übernommen …”, a successful confirmation, or a timeout message. A
confirmation or timeout remains visible for three seconds.

Incoming numeric payloads are parsed strictly. Invalid or only partially
numeric messages are ignored and never overwrite the last confirmed value.

## Device topics

| Topic | Payload | Meaning |
|---|---:|---|
| `pool/display/status` | `online` / `offline` | MQTT availability |
| `pool/display/heartbeat` | `alive` | Heartbeat message |

## Data validity

- Values remain unknown until their first valid status message arrives.
- A malformed payload is ignored and never replaces a confirmed value.
- Retained values represent the last state confirmed by Loxone and remain
  usable while MQTT is connected; cyclic status publishes are not required.
- MQTT disconnect disables command controls immediately.
- If Loxone is unavailable while the broker remains online, an unconfirmed
  command times out after five seconds without changing the displayed state.
- Safety decisions and operating limits always remain in Loxone.
