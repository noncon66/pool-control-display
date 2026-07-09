# MQTT Topics

This document is the communication contract between Loxone and the display.
The display never treats a sent command as confirmed state. It waits until
Loxone publishes the corresponding status topic.

## Status topics (published by Loxone / consumed by the display)

| Topic | Payload | Meaning |
|---|---:|---|
| `pool/status/waterTemp` | `27.4` | Current pool water temperature |
| `pool/status/targetTemp` | `29.0` | Target temperature |
| `pool/status/filterPump` | `0` / `1` | Filter pump state |
| `pool/status/heatingPump` | `0` / `1` | Heating pump state |
| `pool/status/heatingAllowed` | `0` / `1` | Heating permission from heating system |
| `pool/status/isHeating` | `0` / `1` | Pool is currently being heated |
| `pool/status/mode` | `0..2` | Pool operating mode |

Status topics should be published as retained MQTT messages. This allows the
display to receive the latest confirmed Loxone values immediately after a
restart or reconnect.

Mode values:

| Value | Meaning |
|---:|---|
| `0` | Off |
| `1` | Automatic |
| `2` | Manual |

## Command topics (published by the display / consumed by Loxone)

| Topic | Payload | Meaning |
|---|---:|---|
| `pool/cmd/mode` | `0..2` | Set user-selectable pool mode |
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
- The received Loxone data is not stale.

This panel-side check is only user-interface guidance. Loxone remains
authoritative and must independently reject `pool/cmd/filterPump` whenever its
own current operating mode or safety conditions do not permit manual control.

## Device topics

| Topic | Payload | Meaning |
|---|---:|---|
| `pool/display/status` | `online` / `offline` | MQTT availability |
| `pool/display/heartbeat` | `alive` | Heartbeat message |

## Data validity

- Values remain unknown until their first valid status message arrives.
- A malformed payload is ignored and does not refresh the data timestamp.
- Data is marked stale after 60 seconds without any valid status message.
- Stale data may still be displayed for context, but command controls should
  be disabled or clearly marked until communication is current again.
- Safety decisions and operating limits always remain in Loxone.
