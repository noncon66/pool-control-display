# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für die über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Die dunkle, Loxone-inspirierte
GUI ist in LVGL umgesetzt, auf `COM3` geflasht und nach gezielten
Pixelkorrekturen vom Benutzer visuell freigegeben. Als Nächstes stehen die
abschließenden Bedien- und Fehlerzustandstests an.

## Aktueller Git-Stand

- Branch `main`, aktueller Commit `ce98df8` (`Docs: Update final GUI verification
  handoff`), synchron mit dem lokalen Stand von `origin/main`.
- Vor dieser Handoff-Aktualisierung war der Arbeitsbaum sauber; GUI,
  Pixelkorrekturen, Dokumentation und vorheriger Handoff sind vollständig
  committet.
- Eine ignorierte private `include/PoolConfig.h` ist auf diesem Rechner
  vorhanden und bleibt außerhalb von Git.
- Die neue GUI-Firmware wurde auf diesem Rechner erfolgreich auf `COM3`
  geflasht. Die sichtbare Darstellung ist bestätigt; die Bedien- und
  Fehlerzustände sind noch systematisch zu prüfen.

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
- Hardwarekorrekturen nach dem ersten Foto: `AUTOMATIK` verwendet innerhalb
  seines schmaleren Felds 16 statt 20 px, die drei oberen Statuswerte wurden
  fünf Pixel nach oben verschoben und die Titel `Wassertemperatur`,
  `Filterpumpe`, `Heizpumpe`, `Heizfreigabe` sowie `Solltemperatur` von 14 auf
  16 px vergrößert. Der Benutzer bestätigte den korrigierten Stand als final.
- MQTT-Callbacks, Commandwerte, Bestätigungslogik und Bedienpolicy unverändert
  gelassen. Plus/Minus verwenden weiterhin `LV_EVENT_PRESSED`, Modus und
  Filterpumpe weiterhin `LV_EVENT_CLICKED`.
- Veraltete Integrationsaussagen in `README.md`, `docs/architecture.md` und
  `docs/ui.md` korrigiert. Display, GT911, LVGL und `GuiManager::begin()` sind
  im Standardziel integriert; lediglich die neue Gestaltung wartet noch auf
  die Hardwareabnahme.

## Offene Arbeit

- Für die erwogene Akku-/Qi-Erweiterung die reale Stromaufnahme bei aktivem
  Display, ausgeschaltetem Display und WLAN/MQTT-Reconnect mit einem
  USB-Leistungsmesser erfassen. Danach Qi-Empfängerleistung und Akkukapazität
  festlegen.
- Filterpumpe, alle drei Modi sowie Plus/Minus mit der finalen Geometrie noch
  einmal kurz auf Touch und Statusbestätigung prüfen.
- Pending/disabled und MQTT-offline mit der finalen GUI visuell prüfen.
- Heizpumpenstatus `0` bei der nächsten regulären Abschaltung kontrollieren.
- Ursache früherer Schwankungen von `pool/status/targetTemp` prüfen und
  sicherstellen, dass Loxone den stabilen tatsächlich aktiven Sollwert sendet.
- Timeout, MQTT-offline und Reconnect abschließend testen.
- Native Tests erneut ausführen, sobald `gcc/g++` auf dem Host verfügbar ist.
- Veraltete Hinweise in `README.md` und `docs/ui.md`, wonach die visuelle
  Hardwareprüfung noch aussteht, an die bereits erfolgte Freigabe angleichen.

## Wichtige technische Entscheidungen

- Vorläufiges Akku-/Qi-Konzept: geschützter 1S-LiPo ausschließlich an den
  vorhandenen 3,7-V-PH2.0-Anschluss; ein Qi-Empfänger speist als geregelte
  5-V-Quelle den USB-/`5V_IN`-Pfad. Keine zweite LiPo-Ladeschaltung und niemals
  5 V direkt an den Akkuanschluss. Magnetische Ausrichtung als mechanische
  MagSafe-/Qi2-kompatible Halterung behandeln, nicht als Apple-MagSafe-
  Zertifizierung des Eigenbaus.
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
- Auf dem anderen Entwicklungsrechner war mangels Display nur der Build möglich;
  Flash und Hardwareabnahme wurden dort nicht ausgeführt.
- Auf diesem Rechner Hauptziel `esp32-s3-panel` vollständig neu gebaut und auf
  `COM3` geladen: RAM 31,3 % (102548/327680 Bytes), Flash 19,4 %
  (1270430/6553600 Bytes). Bootloader, Partitionen und Firmware wurden
  geschrieben, alle Flash-Hashes verifiziert und das Board per RTS neu
  gestartet. Eine Narrowing-Warnung stammt aus der externen Arduino-GFX-
  Bibliothek und war nicht build- oder upload-blockierend.
- Korrigierte GUI erneut vollständig gebaut und auf `COM3` geladen: RAM 31,3 %
  (102556/327680 Bytes), Flash 19,6 % (1286378/6553600 Bytes); alle
  Flash-Hashes verifiziert. Benutzer bestätigte anschließend, dass
  `AUTOMATIK`, obere Statuswerte und vergrößerte Titel gut aussehen und der
  visuelle Stand unverändert bleiben soll.
- Native Tests nicht ausgeführt; dem Host fehlt weiterhin `gcc/g++`.
- In dieser reinen Statussitzung Arbeitsbaum, Branch, letzte Commits,
  Projektstruktur, `README.md`, `platformio.ini` und relevante offene Hinweise
  geprüft. `git diff --check` war fehlerfrei; Build, Flash und native Tests
  wurden in dieser Sitzung nicht erneut ausgeführt.
- In dieser Beratungssitzung Waveshare-Wiki und Schaltplan sowie das
  AXP2101-Datenblatt geprüft. Bestätigt sind der 1S-PH2.0-Akkuanschluss mit
  Lade-/Entladeverwaltung, der `5V_IN`-Pfad und ein zulässiger AXP2101-VBUS-
  Bereich von 3,9 bis 5,5 V. Keine Hardwaremessung, kein Build und kein Flash
  ausgeführt.

## Nächster konkreter Schritt

Vor einer Akku-/Qi-Auslegung die reale Stromaufnahme des Panels in den drei
relevanten Betriebszuständen messen. Parallel bleibt der nächste Firmware-
Schritt, mit der visuell freigegebenen GUI Filterpumpe, Modi, Plus/Minus,
Pending/disabled, MQTT-offline und Reconnect abschließend zu prüfen.
