# Loxone-MQTT-Simulator

Der Simulator bildet die MQTT-Seite von Loxone für Entwicklungstests nach.
Benötigt werden lediglich ein erreichbarer MQTT-Broker und Python 3.

> **Wichtig:** `192.168.1.10` ist in allen Beispielen nur ein Platzhalter.
> Er muss durch die tatsächliche IP-Adresse oder den Hostnamen deines
> MQTT-Brokers ersetzt werden. Der Simulator startet keinen eigenen Broker.

Der Simulator:

- veröffentlichen alle Statuswerte als retained MQTT-Nachrichten,
- abonnieren alle Panelbefehle unter `pool/cmd/#`,
- bestätigen gültige Befehle mit der passenden Statusmeldung,
- beantworten ungültige Befehle nicht, damit der Fünf-Sekunden-Timeout des
  Panels getestet werden kann,
- verwenden dieselben Tastaturbefehle.

Der Simulator ist nur ein Entwicklungswerkzeug. Er ersetzt und verändert keine
echte Loxone-Konfiguration.

## Lokalen MQTT-Broker auf macOS starten

Für Tests auf einem einzelnen Mac eignet sich Eclipse Mosquitto. Mit Homebrew:

```sh
brew install mosquitto
```

Broker anschließend in einem eigenen Terminal starten:

```sh
mosquitto -v
```

Der Vordergrundmodus `-v` zeigt alle Verbindungen und Nachrichten und lässt
sich mit `Ctrl+C` beenden. Für diesen lokalen Test lauscht der Broker auf
`127.0.0.1:1883`.

In einem zweiten Terminal den Simulator starten:

```sh
python3 ./tools/loxone_mqtt_simulator.py --broker 127.0.0.1
```

Optional können alle Poolnachrichten in einem dritten Terminal beobachtet
werden:

```sh
mosquitto_sub -h 127.0.0.1 -t 'pool/#' -v
```

Der lokale Loopback-Test ist nur für Programme auf demselben Mac erreichbar.
Für einen späteren ESP32-Test muss Mosquitto im lokalen Netzwerk lauschen und
mit Benutzername, Passwort und passenden Firewallregeln abgesichert werden.

## Python 3

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

## Automatischer Broker-Selbsttest

Die Python-Version kann den vollständigen Status- und Befehlsfluss ohne
Tastatureingabe gegen einen erreichbaren Testbroker prüfen:

```sh
python3 ./tools/loxone_mqtt_simulator.py \
  --broker 127.0.0.1 \
  --self-test
```

Der Selbsttest prüft retained Anfangsstatuswerte, gültige Modus-, Pumpen- und
Solltemperaturbefehle sowie die fehlende Bestätigung eines unzulässigen
Solltemperaturbefehls im manuellen Modus. Er beendet sich mit Exitcode `0` bei
Erfolg und einem von null verschiedenen Exitcode bei einem Fehler. GitHub
Actions startet dafür einen isolierten Mosquitto-Broker auf Port `18884`.

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

## Verbindungsprobleme

Die Meldung `MQTT broker ... is not reachable` bedeutet, dass keine
TCP-Verbindung zum Broker aufgebaut werden konnte. Prüfe:

- Ist die angegebene Brokeradresse korrekt?
- Läuft der MQTT-Broker?
- Ist der Port korrekt? Ohne TLS ist das meistens `1883`.
- Befinden sich Computer und Broker im selben erreichbaren Netzwerk?
- Blockiert eine Firewall die Verbindung?
- Benötigt der Broker Benutzername und Passwort?

Ein Verbindungs-Timeout entsteht vor der MQTT-Anmeldung. Falsche Zugangsdaten
führen dagegen zu einer ausdrücklichen Ablehnung durch den Broker.
