# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für die über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Die Betriebsmodus-Schnittstelle
ist jetzt einheitlich auf `1 = Automatik`, `2 = Manuell`, `3 = Aus` korrigiert,
gebaut und auf der Hardware bestätigt.

## Aktueller Git-Stand

- Branch `main`, Ausgangscommit `067688f` (`Docs: Record successful LoxBerry
  command routing tests`).
- Änderungen an Firmware, Simulator, Tests, MQTT-/Loxone-Dokumentation und
  dieser Übergabe sind noch nicht committet.
- Die korrigierte Firmware ist auf `COM3` geflasht.
- Kein Simulator- oder Hilfsprozess läuft.
- Benutzer hat visuell bestätigt, dass bei Statuswert `3` der Button `AUS`
  hervorgehoben wird.
- Nach Neustart wurde `AUTOMATIK` getestet: Das Panel publizierte korrekt
  `pool/cmd/mode = 1`, im seriellen Log jedoch zweimal. Noch klären, ob der
  Benutzer einmal oder zweimal getippt hat.
- Benutzer bestätigt, dass `mode = 1` über MQTT ankommt, aber von Loxone nicht
  verarbeitet wird. Damit liegt der aktuelle Fehler hinter dem MQTT-Publish.
- Loxone-LiveView blieb laut Benutzer auf `3`; der Displaywert `1` kam am
  virtuellen Eingang nicht an. Ein identischer direkter Testpublish wurde
  anschließend zum Vergleich gesendet und kam ebenfalls nicht in Loxone an.
- Ein paralleler Broker-Subscriber empfing den Vergleichsbefehl exakt als
  `pool/cmd/mode = 1`, `retain=false`. Display und Mosquitto sind damit als
  Fehlerquelle ausgeschlossen; die Unterbrechung liegt im LoxBerry MQTT
  Gateway oder dessen HTTP-Weiterleitung zum Miniserver.
- Benutzer bestätigte danach, dass Payload `1` in der Incoming Overview des
  Gateways sichtbar ist, der virtuelle Loxone-Eingang aber auf `3` bleibt.
- Wahrscheinlichste Ursache laut offizieller LoxBerry-Dokumentation: Das
  Gateway cached Werte und überträgt im Normalbetrieb nur Änderungen. Aus den
  früheren Tests kann sein Cache bereits `1` enthalten, obwohl Loxones Eingang
  inzwischen `3` zeigt. `Resend data to Miniserver` erzwingt die Übertragung.
- Benutzer findet `Resend data to Miniserver` in seiner Gateway-Oberfläche
  nicht; die dokumentierte Schaltfläche ist dort offenbar nicht vorhanden.
- Mit Benutzerzustimmung wurde als Cache-Wechsel einmalig
  `pool/cmd/mode = 2`, `retain=false`, erfolgreich an den Broker publiziert.
  Benutzer bestätigte, dass der virtuelle Loxone-Eingang darauf auf `2`
  wechselte. Damit sind Eingang, Authentifizierung und HTTP-Weiterleitung in
  Ordnung; der unveränderte Gateway-Cachewert war die Ursache.
- Anschließend wurde wie vereinbart `pool/cmd/mode = 1`, `retain=false`,
  erfolgreich gesendet, um auf Automatik zurückzustellen. Benutzer bestätigte
  den Wechsel des virtuellen Loxone-Eingangs zurück auf `1`.
- Damit ist Display/Testclient → Mosquitto → LoxBerry Gateway → virtueller
  Loxone-Eingang für echte Wertänderungen end-to-end bestätigt.
- Benutzer möchte `Reset-After-Send` für robuste wiederholte Modusbefehle
  einrichten und hat die Option für `pool/cmd/mode` aktiviert.
- Erster Reset-After-Send-Test wurde mit `pool/cmd/mode = 2`, `retain=false`,
  erfolgreich an den Broker gesendet. Der virtuelle Eingang blieb auf `2`;
  ein Reset auf `0` erfolgte nicht.
- Für absolute Modusbefehle ist die ebenfalls vorhandene Option `Disable Cache`
  technisch besser als `Reset-After-Send`: Jeder identische Wert wird erneut
  weitergeleitet, ohne den absichtlich ungültigen Wert `0` nachzusenden.
- Benutzer hat `Reset-After-Send` ausgeschaltet und `Disable Cache` für
  `pool/cmd/mode` aktiviert. Ein identischer Testwert `2` wurde danach erneut
  erfolgreich an den Broker gesendet; Benutzer bestätigte im Gateway-Log den
  erneuten HTTP-Versand an Loxone. Die Option funktioniert damit wie gewünscht.
- Abschließend wurde `pool/cmd/mode = 1`, `retain=false`, gesendet, um wieder
  auf Automatik zurückzustellen; Benutzer bestätigte den virtuellen Eingang
  auf `1`.
- Read-only Brokerprüfung bestätigte danach `pool/status/mode = 1` mit
  Retain-Flag. Der Hardware-Serielltest zeigte `Mode : Auto` und zunächst
  `Loxone data : CURRENT`.
- Nach 60 Sekunden wurde der Modus wieder `STALE`, weil Loxone den Status noch
  nicht zyklisch erneuert. Der Transport und die Zuordnung sind korrekt; es
  fehlt die periodische Statusveröffentlichung.

## Erledigte Änderungen

- `PoolMode`, MQTT-Payloadparser und Command-Validierung verwenden jetzt
  ausschließlich die Loxone-Werte `1`, `2`, `3`.
- Die UI-Reihenfolge bleibt `Aus`, `Automatik`, `Manuell`; Button-Payload und
  Statushervorhebung verwenden eine explizite Zuordnung statt Arrayindex
  `0..2`.
- Simulator, Parser-Test und Dokumentation wurden auf dieselbe Zuordnung
  umgestellt. Der Simulator-Selbsttest prüft zusätzlich `3 = Aus` und weist den
  veralteten Wert `0` zurück.
- LVGL-Hauptansicht, GT911-Pointer, Screen-Power, sichere Wake-Touch-Sperre und
  policy-geschützte MQTT-Bedienung waren bereits integriert und auf Hardware
  bestätigt.
- LoxBerry leitet `pool/cmd/#` erfolgreich an virtuelle Loxone-Eingänge weiter.

## Offene Arbeit

- Die drei Modusbuttons end-to-end prüfen: Automatik sendet `1`,
  Manuell `2`, Aus `3`; Loxone muss jeweils den tatsächlichen Status retained
  zurückmelden.
- Für den Automatikbefehl kam noch kein `pool/status/mode = 1` zurück; der
  bestätigte Zustand und die Hervorhebung blieben deshalb korrekt auf `Aus`.
- Prüfen, ob der analoge virtuelle Eingang `pool_cmd_mode` noch den früheren
  Testwert `1` hält. Ein erneutes `1` erzeugt dann keine Wertänderung und kann
  eine flankengesteuerte Loxone-Logik nicht auslösen.
- Die übrigen sechs tatsächlichen Zustände als retained Topics publizieren:
  `waterTemp`, `targetTemp`, `filterPump`, `heatingPump`, `heatingAllowed` und
  `isHeating` unter `pool/status/`.
- Danach Rückbestätigung, Timeout, stale/offline und Reconnect end-to-end testen.
- Native Tests erneut ausführen, sobald `gcc/g++` auf dem Host verfügbar ist.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel zeigt keine optimistischen
  Zustände; Commands gelten erst durch ein Statustopic als bestätigt.
- Statusmeldungen sind retained, Befehlstopics nicht.
- `1 = Automatik`, `2 = Manuell`, `3 = Aus` ist der verbindliche Modevertrag.
- Ohne frische Statusdaten bleiben produktive Controls gesperrt.
- Private Gerätewerte liegen nur in der ignorierten `include/PoolConfig.h`.

## Relevante Dateien

- `lib/Pool/PoolState.h`, `lib/Pool/MqttPayloadParser.h` – Modevertrag/Parser
- `lib/Gui/GuiManager.cpp` – Buttonwerte und Statushervorhebung
- `lib/Mqtt/MqttManager.cpp` – Command-Validierung und Publish
- `tools/loxone_mqtt_simulator.py` – Broker-Integrationstest
- `test/test_pool_state/test_main.cpp` – Parser-/Zustandstests
- `docs/mqtt.md`, `docs/loxone.md` – verbindliche Integration

## Tatsächlich ausgeführte Prüfungen

- Firmware-Releasebuild `esp32-s3-panel` erfolgreich: RAM 31,3 %, Flash 19,0 %.
- Firmware erfolgreich auf `COM3` geladen; alle Flash-Hashes verifiziert.
- Serieller Hardwaretest nach dem Flash: retained `pool/status/mode = 3` wurde
  empfangen und als `Mode : Off` ausgegeben.
- Benutzer bestätigte die dazu passende grüne Hervorhebung von `AUS`.
- Erster Tipp auf `AUTOMATIK` wurde vom Touchcontroller erkannt, aber korrekt
  nicht publiziert: `Loxone data : STALE`. Der nur beim MQTT-Verbindungsaufbau
  empfangene retained Modus war bereits älter als 60 Sekunden.
- Nach dem anschließenden Neustart wurde retained `mode = 3` wieder als frisch
  eingelesen. `AUTOMATIK` publizierte korrekt Payload `1`, allerdings zweimal;
  eine Status-Rückmeldung `mode = 1` wurde im Messfenster nicht empfangen.
- Simulator mit der PlatformIO-Python-Umgebung bis zur CLI-Hilfe gestartet;
  Imports und Syntax sind damit gültig. Broker-Selbsttest nicht erneut
  ausgeführt.
- Direkter Vergleichstest erfolgreich an den Broker publiziert:
  `pool/cmd/mode = 1`, `retain=false`. Ob LoxBerry ihn an Loxone weiterleitete,
  wurde verneint.
- Zweiter MQTT-Client hat denselben Publish direkt vom Broker empfangen:
  Topic `pool/cmd/mode`, Payload `1`, Retain-Flag `false`.
- `git diff --check` vor der finalen Übergabe ohne Fehler.
- Native Tests nicht ausgeführt; dem Host fehlt weiterhin `gcc/g++`.

## Nächster konkreter Schritt

In Loxone `pool/status/mode` spätestens alle 30 Sekunden retained erneut
publizieren, damit der Displaywert nicht nach 60 Sekunden stale wird. Danach
visuell `AUTOMATIK` hervorheben lassen und die drei Displaybuttons samt
Statusbestätigung end-to-end testen.
