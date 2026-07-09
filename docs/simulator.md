# Loxone-MQTT-Simulator

Der Simulator bildet die MQTT-Seite von Loxone für Entwicklungstests nach.
Benötigt werden lediglich ein erreichbarer MQTT-Broker und entweder Python 3
oder PowerShell.

Beide Varianten:

- veröffentlichen alle Statuswerte als retained MQTT-Nachrichten,
- abonnieren alle Panelbefehle unter `pool/cmd/#`,
- bestätigen gültige Befehle mit der passenden Statusmeldung,
- beantworten ungültige Befehle nicht, damit der Fünf-Sekunden-Timeout des
  Panels getestet werden kann,
- verwenden dieselben Tastaturbefehle.

Der Simulator ist nur ein Entwicklungswerkzeug. Er ersetzt und verändert keine
echte Loxone-Konfiguration.

## Python 3 – empfohlen für macOS

Datei: `tools/loxone_mqtt_simulator.py`

Die Python-Version läuft unter macOS, Linux und Windows und verwendet
ausschließlich die Python-Standardbibliothek. Eine Installation mit `pip` ist
nicht erforderlich.

### Start unter macOS oder Linux

```sh
python3 ./tools/loxone_mqtt_simulator.py --broker 192.168.1.10
```

### Start unter Windows

```powershell
python .\tools\loxone_mqtt_simulator.py --broker 192.168.1.10
```

### Mit MQTT-Zugangsdaten

```sh
python3 ./tools/loxone_mqtt_simulator.py \
  --broker 192.168.1.10 \
  --username mqtt-user \
  --password mqtt-password
```

## PowerShell-Alternative

Datei: `tools/loxone_mqtt_simulator.ps1`

Windows enthält Windows PowerShell. Unter macOS muss PowerShell 7 (`pwsh`)
separat installiert sein.

### Start unter Windows

```powershell
.\tools\loxone_mqtt_simulator.ps1 -Broker 192.168.1.10
```

### Start unter macOS

```sh
pwsh ./tools/loxone_mqtt_simulator.ps1 -Broker 192.168.1.10
```

### Mit MQTT-Zugangsdaten

```powershell
.\tools\loxone_mqtt_simulator.ps1 `
  -Broker 192.168.1.10 `
  -Username mqtt-user `
  -Password mqtt-password
```

Zugangsdaten gelten nur für den laufenden Prozess. Echte Zugangsdaten dürfen
nicht in den Skripten oder im Repository gespeichert werden.

## Simulierte Loxone-Regeln

- Die Betriebsmodi `0`, `1` und `2` werden akzeptiert.
- Die Solltemperatur wird nur im Automatikmodus angenommen.
- Zulässige Solltemperatur: 20,0 bis 32,0 °C in Schritten von 0,5 °C.
- Filterpumpenbefehle werden nur im manuellen Modus angenommen.
- `isHeating` ist ein unabhängiger Status und kein Betriebsmodus.

## Tastaturbefehle

| Taste | Aktion |
|---|---|
| `A` | Automatikmodus veröffentlichen |
| `M` | Manuellen Modus veröffentlichen |
| `O` | Modus Aus veröffentlichen |
| `H` | `isHeating` umschalten |
| `F` | Filterpumpenstatus umschalten |
| `P` | Alle Statuswerte erneut veröffentlichen |
| `Q` | Simulator beenden |

Die Tastatursteuerung benötigt ein normales interaktives Terminal. Sie ist
nicht für umgeleitete Standardeingabe vorgesehen.

## Typische Tests

### Erfolgreiche Solltemperaturänderung

1. Mit `A` den Automatikmodus setzen.
2. Vom Panel eine gültige Solltemperatur senden.
3. Der Simulator veröffentlicht den neuen Wert auf
   `pool/status/targetTemp`.

### Abgelehnte Solltemperaturänderung

1. Mit `M` den manuellen Modus setzen.
2. Vom Panel eine Solltemperatur senden.
3. Der Simulator antwortet nicht; das Panel erreicht nach fünf Sekunden den
   Bestätigungs-Timeout.

### Manuelle Filterpumpe

1. Mit `M` den manuellen Modus setzen.
2. Filterpumpe über das Panel schalten.
3. Der Simulator bestätigt über `pool/status/filterPump`.
