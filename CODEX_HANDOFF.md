# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für die über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Display, Touch, Screen-Power,
MQTT-Publishes und die Weiterleitung zu den virtuellen Loxone-Eingängen sind
bestätigt. Als Nächstes wird die Rückrichtung Loxone → retained MQTT-Status →
Panel eingerichtet.

## Aktueller Git-Stand

- Branch `main`, Ausgangscommit `75b8b5a` (`MQTT: Enable policy-guarded display
  controls`).
- Nur `CODEX_HANDOFF.md` ist durch die laufende Dokumentation geändert.
- Normale Fünf-Minuten-Firmware mit aktivierter, policy-geschützter
  MQTT-Bedienung ist auf `COM3` geflasht.
- Kein Simulator- oder Hilfsprozess läuft; keine temporären Skripte vorhanden.

## Erledigte Änderungen

- LVGL-Hauptansicht, GT911-Pointer und sichere Wake-Touch-Sperre sind auf
  Hardware integriert und bestätigt.
- `GuiManager` läuft mit `controlsEnabled=true`; Offline-, Stale-, Unknown- und
  Modusregeln aus `PanelViewModel`/`PanelControlPolicy` bleiben verbindlich.
- `MqttManager` publiziert erfolgreiche Commands mit Topic/Payload im Log.
  Befehlstopics sind nicht retained.
- `WifiManager` nutzt `WiFi.persistent(false)`, um Flash-Cache-Stalls während
  kontinuierlicher RGB-DMA zu vermeiden.
- In Loxone wurden die virtuellen Eingänge angelegt. Das LoxBerry MQTT Gateway
  leitet `pool/cmd/#` per HTTP Virtual Inputs zum Miniserver weiter.

## Offene Arbeit

- In Loxone einen virtuellen UDP-Ausgang zum LoxBerry MQTT Gateway einrichten.
- Sieben tatsächliche Zustände als retained Topics publizieren:
  `waterTemp`, `targetTemp`, `filterPump`, `heatingPump`, `heatingAllowed`,
  `isHeating` und `mode` unter `pool/status/`.
- Danach echtes End-to-End testen: Rückbestätigung, Timeout, stale/offline und
  Reconnect.
- Die Loxone-Logik muss Moduswerte auf exakt `0`, `1`, `2` begrenzen sowie
  Solltemperatur und Filterpumpe anhand Modus/Sicherheitslogik validieren.
- Native Tests erneut ausführen, sobald `gcc/g++` verfügbar ist.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel zeigt keine optimistischen
  Zustände; Commands gelten erst durch ein Statustopic als bestätigt.
- Statusmeldungen sind retained, Befehlstopics nicht.
- Ohne frische Statusdaten bleiben produktive Controls gesperrt.
- Private Gerätewerte liegen nur in der ignorierten `include/PoolConfig.h`.

## Relevante Dateien

- `lib/Mqtt/MqttManager.*`, `lib/Mqtt/Topics.h` – Publish, Status und Topics
- `lib/Gui/GuiManager.*`, `lib/Pool/PanelViewModel.h` – UI und Freigaben
- `lib/Pool/PanelControlPolicy.h` – Modus-/Freshness-Regeln
- `docs/loxone.md`, `docs/mqtt.md` – LoxBerry-/Loxone-Konfiguration
- `tools/loxone_mqtt_simulator.py` – Broker-Integrationstest

## Tatsächlich ausgeführte Prüfungen

- Firmware gebaut, mit Hash-Prüfung auf `COM3` geladen und stabil gestartet.
- Alle drei Panelbefehle zweimal gegen den Simulator bestätigt: Solltemperatur,
  Betriebsmodus und Filterpumpe inklusive retained Status-Rückmeldung.
- Gateway → Loxone-Eingänge manuell getestet:
  - `pool/cmd/mode`: Werte `1`, `2` und mehrfach absichtlich ungültig `3`
  - `pool/cmd/filterPump`: Werte `1` und `0`
  - `pool/cmd/targetTemp`: Werte `29.5` und `28.0`
- Alle Testpublishes waren einmalig und nicht retained.
- Benutzer bestätigte abschließend, dass alle drei virtuellen Eingangswege gut
  funktionieren.
- Simulierte retained Statuswerte wurden nach den Broker-Tests gelöscht.
- Native Tests nicht ausgeführt; dem Host fehlt weiterhin `gcc/g++`.

## Nächster konkreter Schritt

Den virtuellen UDP-Ausgang vom Miniserver zum LoxBerry MQTT Gateway anlegen und
zuerst `pool/status/mode` mit dem tatsächlich aktiven, normalisierten Modus als
retained Status veröffentlichen. Danach Empfang und UI-Anzeige am Panel prüfen.
