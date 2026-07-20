# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für die über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Die freigegebene dunkle,
Loxone-inspirierte GUI ist in LVGL umgesetzt und lokal gebaut. Als Nächstes
steht die Prüfung auf dem realen 480×480-Display an.

## Aktueller Git-Stand

- Branch `main`, Basis-Commit `addaef8` (`Docs: Record approved GUI design
  direction`), synchron mit `origin/main`.
- Lokale, noch nicht committete Änderungen: `GuiManager.cpp/.h`, `README.md`,
  `docs/architecture.md`, `docs/ui.md` und dieser Handoff.
- Für den lokalen Build wurde eine ignorierte `include/PoolConfig.h` mit
  Platzhalterwerten angelegt; sie enthält keine produktiven Zugangsdaten und
  wird nicht committet.
- Das Display ist aktuell nicht verfügbar. Die neue GUI wurde deshalb weder
  geflasht noch visuell oder per Touch auf Hardware geprüft.

## Erledigte Änderungen

- Den vom Benutzer freigegebenen Entwurf
  `pool-control-loxone-draft.html` in `lib/Gui/GuiManager.cpp` übertragen.
- Konsequenter Dark Mode mit ruhigen Karten und Loxone-inspiriertem Grün als
  Akzent umgesetzt.
- Wassertemperatur als kompakte horizontale Karte ausgeführt; der Wert sitzt
  vertikal stabil und wird nicht mehr an der Unterkante abgeschnitten.
- Filterpumpen-Kachel als tatsächliches Bedienelement erkennbar gemacht:
  Power-Symbol, `SCHALTEN`-Badge, Pfeil, eigener Button-Kontrast und aktiver
  Zustand.
- Betriebsmodus deutlich höher und prominenter gestaltet; die drei Tasten
  besitzen Symbole und eine grüne aktive Auswahl.
- Plus/Minus-Tasten höher gestaltet und mit großen, kräftigen Zeichen versehen.
- Statuskarten für Heizpumpe und Heizfreigabe sowie Kopf- und Fußzeile an die
  neue visuelle Hierarchie angepasst.
- MQTT-Callbacks, Commandwerte, Bestätigungslogik und Bedienpolicy unverändert
  gelassen. Plus/Minus verwenden weiterhin `LV_EVENT_PRESSED`, Modus und
  Filterpumpe weiterhin `LV_EVENT_CLICKED`.
- Veraltete Integrationsaussagen in `README.md`, `docs/architecture.md` und
  `docs/ui.md` korrigiert. Display, GT911, LVGL und `GuiManager::begin()` sind
  im Standardziel integriert; lediglich die neue Gestaltung wartet noch auf
  die Hardwareabnahme.

## Offene Arbeit

- Sobald das Display wieder verfügbar ist: Firmware flashen und Layout,
  Schriftgrößen, Kontrast, Touch-Ziele sowie alle aktiven/disabled Zustände auf
  dem realen Panel prüfen; danach nötige Pixelkorrekturen vornehmen.
- Heizpumpenstatus `0` bei der nächsten regulären Abschaltung kontrollieren.
- Ursache früherer Schwankungen von `pool/status/targetTemp` prüfen und
  sicherstellen, dass Loxone den stabilen tatsächlich aktiven Sollwert sendet.
- Timeout, MQTT-offline und Reconnect abschließend testen.
- Native Tests erneut ausführen, sobald `gcc/g++` auf dem Host verfügbar ist.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel übernimmt Commands erst nach
  einer bestätigenden Statusmeldung.
- Modusvertrag: `1 = Automatik`, `2 = Manuell`, `3 = Aus`; `0` wird abgelehnt.
- Statusmeldungen sind retained, Befehlstopics nicht.
- Bekannte retained Werte bleiben bei bestehender MQTT-Verbindung bedienbar;
  unbekannte Werte und MQTT-offline sperren die betroffenen Controls.
- Private Gerätewerte liegen nur in der ignorierten `include/PoolConfig.h`.
- `isHeating` bleibt entfernt; Heizpumpe und Heizfreigabe sind die verbindlichen
  Heizstatuswerte.
- Die explizite 480×480-Geometrie bleibt bis zur Hardwareabnahme bestehen, da
  das Zielpanel eine feste Auflösung besitzt.

## Relevante Dateien

- `lib/Gui/GuiManager.cpp`, `lib/Gui/GuiManager.h` – neue LVGL-Gestaltung
- `lib/Pool/PoolState.h`, `lib/Pool/MqttPayloadParser.h` – Zustand/Modusvertrag
- `lib/Mqtt/MqttManager.cpp` – Command-Validierung und Publish
- `README.md`, `docs/architecture.md`, `docs/ui.md` – aktueller Projekt-/GUI-Stand
- `docs/mqtt.md`, `docs/loxone.md` – verbindliche MQTT-/Loxone-Integration
- `tools/loxone_mqtt_simulator.py` – Broker-Integrationstest

## Tatsächlich ausgeführte Prüfungen

- Hauptziel `esp32-s3-panel` mit
  `C:\Users\NUK\.platformio\penv\Scripts\pio.exe run -e esp32-s3-panel -j 2`
  kompiliert und gelinkt. Erzeugt wurden `firmware.bin` (1.270.896 Bytes) und
  `firmware.elf` (16.692.056 Bytes).
- ELF-Größe mit `xtensa-esp32s3-elf-size.exe` geprüft: Text 999.818 Bytes,
  Data 293.676 Bytes, BSS 2.306.873 Bytes.
- Der erste Aufruf über `pio` scheiterte, weil PlatformIO nicht im `PATH` lag.
  Ein Sandbox-Lauf scheiterte am Zugriff auf `platforms.lock`; der Build wurde
  danach außerhalb der Sandbox ausgeführt. Ein weiterer Lauf zeigte zunächst
  die erwartete fehlende private `PoolConfig.h`; danach wurde nur für den
  lokalen Build die ignorierte Platzhalterdatei angelegt.
- `git diff --check` nach Code-, Dokumentations- und Handoff-Änderungen
  fehlerfrei.
- Kein Flash, kein Hardware-, Touch- oder visueller Paneltest ausgeführt, weil
  das Display nicht verfügbar ist.
- Native Tests nicht ausgeführt; dem Host fehlt weiterhin `gcc/g++`.

## Nächster konkreter Schritt

Wenn das Display wieder verfügbar ist, das aktuelle Hauptziel flashen und die
neue GUI auf dem Panel systematisch prüfen: zuerst Geometrie und Lesbarkeit,
dann Filterpumpe, alle drei Modi, Plus/Minus, Pending/disabled und
MQTT-offline. Anschließend nur die auf Hardware erkennbaren Detailabweichungen
nachjustieren.
