# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für die über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Display, Touch, Screen-Power
und alle drei MQTT-Befehlswege sind gegen den LoxBerry-Broker bestätigt. Die
Weiterleitung vom MQTT Gateway zum Miniserver ist noch nicht eingerichtet und
bildet den nächsten Integrationsschritt.

## Aktueller Git-Stand

- Branch `main`, Ausgangscommit `2b500ae` (`Display: Integrate LVGL touch input
  and safe wake handling`).
- Uncommittete Änderungen in `AppController.cpp`, `DisplayManager.cpp`,
  `MqttManager.cpp` und `CODEX_HANDOFF.md`.
- Normale Fünf-Minuten-Firmware mit aktivierter, policy-geschützter
  MQTT-Bedienung ist auf `COM3` geflasht.
- Kein Simulatorprozess läuft; temporäre Testskripte und retained Testwerte
  wurden entfernt.

## Erledigte Änderungen

- `AppController` startet `GuiManager` mit `controlsEnabled=true`.
- Produktive Controls verwenden ausschließlich die zentrale
  Offline-/Stale-/Unknown-/Modus-Freigabe aus `PanelViewModel` und
  `PanelControlPolicy`.
- `MqttManager::publishCommand()` protokolliert erfolgreiche Publishes mit
  Topic und Payload. Befehlstopics bleiben nicht retained.
- Boot-Logs melden `MQTT controls=enabled`; die veraltete Disabled-Meldung im
  Displaytreiber wurde entfernt.
- Wake-Sperre bleibt unverändert: erster Touch aus Off weckt nur, erst der
  nächste Touch darf LVGL und damit MQTT erreichen.

## Offene Arbeit

- LoxBerry MQTT Gateway für `pool/cmd/#` zum Miniserver konfigurieren.
- Loxone so konfigurieren, dass alle sieben `pool/status/#`-Werte retained
  über LoxBerry publiziert werden.
- Danach echtes End-to-End testen: Solltemperatur in Automatik, Filterpumpe in
  Manuell, Betriebsmodus, Bestätigung, Timeout, stale/offline und Reconnect.
- Native Tests erneut ausführen, sobald `gcc/g++` verfügbar ist.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel zeigt keine optimistischen
  Zustände; Befehle gelten erst durch ein Statustopic als bestätigt.
- Statusmeldungen sind retained, Befehlstopics nicht.
- Ohne frische Statusdaten bleiben produktive Controls gesperrt, auch wenn der
  MQTT-Broker verbunden ist.
- WLAN bleibt nichtpersistent (`WiFi.persistent(false)`), um Flash-Cache-Stalls
  während kontinuierlicher RGB-DMA zu vermeiden.
- Private Gerätewerte liegen nur in der ignorierten `include/PoolConfig.h`.

## Relevante Dateien

- `lib/Core/AppController.cpp` – Aktivierung der produktiven GUI-Callbacks
- `lib/Mqtt/MqttManager.*`, `lib/Mqtt/Topics.h` – Publish, Status und Topics
- `lib/Gui/GuiManager.*`, `lib/Pool/PanelViewModel.h` – UI und Freigaben
- `lib/Pool/PanelControlPolicy.h` – Modus-/Freshness-Regeln
- `tools/loxone_mqtt_simulator.py` – retained Status und Befehlsbestätigung
- `docs/mqtt.md`, `docs/loxone.md`, `docs/simulator.md` – Protokoll und Test

## Tatsächlich ausgeführte Prüfungen

- Firmware erfolgreich gebaut: 102.532 Byte RAM (31,3 %) und 1.247.374 Byte
  Flash (19,0 %), anschließend erfolgreich mit Hash-Prüfung auf COM3 geladen.
- Simulator veröffentlichte sieben retained Statuswerte; Panel empfing sie und
  meldete `Loxone data: CURRENT`.
- Plus-Touch veröffentlichte nicht retained `pool/cmd/targetTemp = 29.5`;
  Simulator bestätigte retained `pool/status/targetTemp = 29.5`, UI aktualisiert.
- `MANUELL` veröffentlichte nicht retained `pool/cmd/mode = 2`; Simulator
  bestätigte retained `pool/status/mode = 2`, UI wechselte zu Manuell.
- Filterpumpen-Touch veröffentlichte nicht retained
  `pool/cmd/filterPump = 1`; Simulator bestätigte retained
  `pool/status/filterPump = 1`, UI zeigte EIN.
- Simulatorlog bestätigte alle drei empfangenen Command-Topics und die jeweils
  zurückgesendeten Statuswerte.
- Der vollständige Drei-Befehle-Test wurde auf Benutzerwunsch ein zweites Mal
  wiederholt und erneut bestanden: Soll 29,0→29,5 °C, Automatik→Manuell und
  Filterpumpe AUS→EIN, jeweils Publish plus Statusbestätigung.
- Simulator beendet und alle sieben retained Teststatuswerte mit leeren
  retained Publishes vom LoxBerry-Broker gelöscht. Auch nach dem zweiten Test
  wurden Simulator und retained Testwerte wieder entfernt.
- Panel danach neu gestartet: WLAN und MQTT verbunden, keine simulierten
  Statuswerte empfangen, `Loxone data: UNKNOWN`; Cleanup damit bestätigt.
- `git diff --check` war vor Build und abschließender Prüfung erfolgreich.
- Native Tests nicht ausgeführt; dem Host fehlt weiterhin `gcc/g++`.

## Nächster konkreter Schritt

Die MQTT-Gateway-Subscription `pool/cmd/#` und die zugehörigen virtuellen
Loxone-Eingänge einrichten. Danach die sieben retained Statustopics vom
Miniserver über LoxBerry publizieren und denselben Drei-Befehle-Test ohne
Simulator wiederholen.
