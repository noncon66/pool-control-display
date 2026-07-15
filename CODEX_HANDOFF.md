# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für eine über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Die Entwicklungsumgebung soll
zunächst reproduzierbar auf einen neuen Windows-Rechner umziehen. Danach ist
der nächste Hardware-Meilenstein die sichere Erstprüfung des als
ESP32-S3-Touch-LCD-4B Rev2.2 identifizierten Panels: Werksdemo dokumentieren
und anschließend ausschließlich den isolierten Display-Bring-up testen.

## Aktueller Git-Stand

- Branch `main`, vor dieser Handoff-Aktualisierung sauber und laut
  `git status --short --branch` identisch mit `origin/main`.
- `AGENTS.md` und `CODEX_HANDOFF.md` sind versioniert.
- Vor dieser Aktualisierung war der Arbeitsbaum sauber.
- Aktueller ungestagter Diff: nur diese Aktualisierung von `CODEX_HANDOFF.md`.
  Im Index liegen keine Änderungen.
- Es wurde in dieser Sitzung nichts committet.

## Bereits erledigt

- Wi-Fi-/MQTT-Reconnect, validiertes Statusmodell, strikte Payload-Auswertung,
  Befehlsbestätigung und -timeouts, Screen-Power-Policy, serielles
  Diagnose-Dashboard sowie optionales, standardmäßig deaktiviertes OTA sind
  implementiert.
- `PoolStatusUpdater` trennt Topic-/Payload-Verarbeitung von Arduino, Netzwerk
  und Broker. Die native Testsuite enthält 31 registrierte Unity-Tests für
  Status, Frische, Berechtigungen, Befehle und relevante Randfälle.
- Der Python-MQTT-Simulator besitzt einen nichtinteraktiven Broker-Selbsttest.
  CI baut die Firmware, führt die nativen Tests aus und prüft Status-, Befehls-
  und Retain-Vertrag gegen einen isolierten Mosquitto-Broker.
- Die Broker-Synchronisation des Selbsttests verwendet PINGREQ/PINGRESP, damit
  retained QoS-0-Publishes vor dem Prüfabonnement verarbeitet sind.
- Die reale Integration ist als LoxBerry MQTT Gateway dokumentiert:
  `pool/cmd/#` gelangt über HTTP Virtual Inputs zu Loxone; bestätigte Zustände
  werden von Loxone über UDP `11884` mit `retain` an LoxBerry veröffentlicht.
  Die native MQTT-Integration des Miniservers wird nicht verwendet.
- Dashboard-Konzept, Architektur, MQTT-Vertrag, Simulator, CI,
  Hardware-Bring-up und LoxBerry-Anbindung sind dokumentiert.
- Die redundanten PowerShell-Werkzeuge wurden entfernt. `display_bringup.py`
  und `loxone_mqtt_simulator.py` sind die einzigen gepflegten Tool-Frontends.
- Der CI-Parsefehler in `platformio.ini` wurde behoben: Das versehentliche
  Präfix `Docs` vor `[platformio]` wurde entfernt.
- Das gelieferte Panel wurde anhand des Fotos als
  `ESP32-S3-Touch-LCD-4B Rev2.2` dokumentiert. Die aktuelle offizielle
  Waveshare-Wiki, das aktuelle Schaltbild und der offizielle BSP wurden
  gegengeprüft. Deren Display-Pins, I2C-Pins, Backlight-Pin, TCA9554-Adresse
  und Expander-Resetfolge stimmen mit `src/display_bringup.cpp` überein.

## Offene Arbeit

- Die dokumentierte LoxBerry-/Loxone-Zuordnung an der realen Installation
  einrichten und prüfen: drei virtuelle Befehlseingänge, UDP-Ausgang auf Port
  `11884`, sieben retained Statustopics und zyklische Aktualisierung spätestens
  alle 30 Sekunden.
- Optional einen standardmäßig passiven LoxBerry-Vertragstest ergänzen, der
  retained Topics, Payloadformate und Aktualisierungsintervalle gegen den
  realen Broker prüft. Reale Befehle dürfen nur nach expliziter Freigabe
  gesendet werden.
- Das Waveshare-Panel zuerst unverändert mit der Werksdemo prüfen, Touch und
  Bildschirm dokumentieren, den erkannten seriellen Port festhalten und danach
  den isolierten Python-Bring-up gegen den Hersteller-Demoaufbau testen.
- Danach ST7701-Display, GT911-Touch und Backlight in die normale Firmware
  portieren, `ScreenPowerPolicy` anbinden und auf echter Hardware testen.
- Anschließend LVGL aktivieren und Wartungs-/Einstellungsansichten ergänzen.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel zeigt nur bestätigte
  MQTT-Statuswerte und veröffentlicht Benutzeranforderungen.
- Keine optimistischen UI-Updates: Ein Befehl gilt erst nach einer passenden
  Loxone-Statusmeldung als bestätigt. Unbekannte, stale oder offline Daten
  deaktivieren die betroffenen Bedienelemente.
- Die Kommunikation läuft über Mosquitto und das MQTT Gateway auf LoxBerry,
  nicht über das native MQTT-Plugin des Miniservers.
- Statusmeldungen sind retained; Befehlstopics sind nicht retained. Für
  Bedienfreigaben relevante Werte müssen regelmäßig aktualisiert werden.
- Hardwareunabhängige Poollogik bleibt von Hardware- und Netzwerkcode getrennt
  und wird im nativen PlatformIO-Ziel getestet.
- Display-Support bleibt im Standard-Build deaktiviert, bis ST7701 und GT911
  auf echter Hardware verifiziert sind. Dafür existiert das isolierte Ziel
  `esp32-s3-display-bringup`.
- Der PCB-Aufdruck `Rev2.2` wird nicht mit der vom offiziellen BSP genannten
  Hardwarefamilie `V1.0` gleichgesetzt. Da die aktuelle offizielle Belegung mit
  dem Bring-up-Code übereinstimmt, ist Rev2.2 kein belegter Inkompatibilitätsgrund;
  vor dem ersten eigenen Flash bleibt die unveränderte Werksdemo maßgeblich.
- Private Gerätewerte liegen nur in der ignorierten Datei
  `include/PoolConfig.h`; OTA ist standardmäßig deaktiviert.
- Python ist die alleinige Implementierung für Simulator und Display-Bring-up;
  keine parallelen PowerShell-Versionen pflegen.
- Beim Rechnerwechsel das Repository frisch von GitHub klonen und generierte
  `.pio`-/VS-Code-Dateien nicht übertragen. Die ignorierte private Datei
  `include/PoolConfig.h` muss separat sicher übertragen oder aus
  `PoolConfig.example.h` neu angelegt werden.
- `CODEX_HANDOFF.md` ist gemäß `AGENTS.md` vor jeder Sitzung zu lesen und am
  Sitzungsende kompakt auf den tatsächlichen Stand zu aktualisieren.

## Relevante Dateien

- `AGENTS.md`, `CODEX_HANDOFF.md` – verbindlicher Übergabeprozess und aktueller
  Arbeitsstand
- `README.md`, `docs/architecture.md` – Projektstatus und Architektur
- `docs/mqtt.md`, `docs/loxone.md`, `docs/simulator.md` – MQTT-Vertrag,
  LoxBerry-Umsetzung und Simulator
- `docs/hardware.md`, `docs/ui.md` – Hardware- und UI-Konzept
- `platformio.ini` – Firmware-, Native-Test- und Display-Bring-up-Ziele
- `lib/Pool/PoolStatusUpdater.h`, `lib/Pool/PoolState.h` – nativ testbare
  Statusverarbeitung
- `lib/Mqtt/MqttManager.*` – MQTT-Transport, Befehle und Reconnect
- `tools/loxone_mqtt_simulator.py`, `tools/display_bringup.py` – gepflegte
  Python-Werkzeuge
- `src/main.cpp`, `src/display_bringup.cpp` – Firmware und Hardware-Smoke-Test
- `test/test_pool_state/test_main.cpp` – 31 native Unity-Tests
- `.github/workflows/ci.yml`, `.github/mosquitto-ci.conf` – automatischer Build,
  native Tests und MQTT-Broker-Integration

## Tatsächlich ausgeführte Prüfungen

- Git-Status, Commit-Historie, Tracking-Diff und Staging-Diff geprüft. Vor der
  Handoff-Aktualisierung war `main` sauber und identisch mit `origin/main`.
- Vor dem Commit `7d2d492` wurden beide verbleibenden Python-Werkzeuge mit
  `py_compile` geprüft und ihre `--help`-Aufrufe erfolgreich ausgeführt;
  verwaiste `.ps1`-/PowerShell-Skriptverweise wurden gesucht.
- Der Benutzer meldete den GitHub-Actions-Lauf nach dem MQTT-
  Synchronisationsfix als erfolgreich. Für den aktuellen Handoff-Stand wurde
  in dieser Sitzung kein neuer Firmware-, Native- oder Broker-Test gestartet.
- Für die Hardware-Erstprüfung wurden Handoff, Hardwaredokumentation,
  Bring-up-Launcher, isolierter Smoke-Test, PlatformIO-Ziel und Boarddefinition
  gelesen. Es wurde noch kein Hardwaretest ausgeführt.
- In dieser Sitzung wurden `CODEX_HANDOFF.md` und `AGENTS.md` gelesen. Der
  Git-Stand wurde mit `git status --short --branch`, `git rev-parse --short
  HEAD` und `git log -1 --oneline` geprüft; die Befehle lieferten die erwarteten
  Daten, aber macOS meldete im Sandbox-Kontext eine `xcrun`-Cache-Warnung.
  Es wurden keine Firmware-, Native-, Broker- oder Hardwaretests gestartet.
- Für den gemeldeten CI-Fehler wurde `platformio.ini` gelesen und korrigiert.
  Die Datei wurde mit `python3 -c 'import configparser; ...'` erfolgreich
  geparst. `pio project config` wurde erfolgreich mit
  `PLATFORMIO_CORE_DIR=/private/tmp/pio-core-pool-control-display` ausgeführt
  und listete alle drei Environments. Ein erster `pio project config` ohne
  temporäres Core-Verzeichnis gab die Konfiguration ebenfalls aus, meldete aber
  beim Beenden eine Sandbox-/Home-Verzeichnis-Warnung für
  `~/.platformio/.cache`. Es wurden keine Firmware-, Native-, Broker- oder
  Hardwaretests gestartet.
- In der aktuellen Sitzung wurden `CODEX_HANDOFF.md`, `README.md`,
  `platformio.ini`, `.gitignore`, `PoolConfig.example.h`, die Hardware-, CI-
  und Simulatordokumentation, der Bring-up-Launcher, die Boarddefinition sowie
  Git-Status und Remote geprüft. Daraus wurde eine Windows-Umzugs- und
  Verifikationscheckliste abgeleitet. Es wurden keine Firmware-, Native-,
  Broker- oder Hardwaretests ausgeführt und nichts geflasht.

## Nächster konkreter Schritt

Den aktuellen Arbeitsstand einschließlich `CODEX_HANDOFF.md` committen und zu
GitHub pushen. Auf dem neuen Windows-Rechner Git, VS Code mit PlatformIO IDE
und Python 3 installieren, das Repository frisch klonen, die private
`include/PoolConfig.h` sicher wiederherstellen und Firmware-Build sowie native
Tests prüfen. Danach das Panel über `USB TO UART` mit unveränderter Werksdemo
prüfen und erst anschließend den isolierten Display-Bring-up verwenden.
