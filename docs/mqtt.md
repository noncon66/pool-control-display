# MQTT Topics

## Status topics (published by Loxone / consumed by the display)

| Topic | Payload | Meaning |
|---|---:|---|
| `pool/status/waterTemp` | `27.4` | Current pool water temperature |
| `pool/status/targetTemp` | `29.0` | Target temperature |
| `pool/status/filterPump` | `0` / `1` | Filter pump state |
| `pool/status/heatingPump` | `0` / `1` | Heating pump state |
| `pool/status/heatingAllowed` | `0` / `1` | Heating permission from heating system |
| `pool/status/mode` | `0..3` | Pool mode |

## Command topics (published by the display / consumed by Loxone)

| Topic | Payload | Meaning |
|---|---:|---|
| `pool/cmd/mode` | `0..3` | Set pool mode |
| `pool/cmd/targetTemp` | float | Set target temperature |
| `pool/cmd/filterPump` | `0` / `1` | Manually switch filter pump |

## Device topics

| Topic | Payload | Meaning |
|---|---:|---|
| `pool/display/status` | `online` / `offline` | MQTT availability |
| `pool/display/heartbeat` | `alive` | Heartbeat message |