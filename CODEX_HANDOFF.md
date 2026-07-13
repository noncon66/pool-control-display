# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für Loxone/LoxBerry
fertigstellen. Der hardwareunabhängige Kern und die Integrationsdokumentation
sind vorhanden; als nächster technischer Meilenstein steht die Inbetriebnahme
von Display und Touch auf dem realen Waveshare-Panel an.

## Aktueller Git-Stand

- Branch `main`, `HEAD` `678ffd9` (`docs: document LoxBerry MQTT integration`),
  identisch mit `origin/main`.
- Kein Diff an versionierten Dateien und keine Änderungen im Index.
- Unversioniert: `AGENTS.md` und `CODEX_HANDOFF.md`. Es wurde nichts committet.

## Bereits erledigte Änderungen

- Der versionierte Stand enthält Wi-Fi-/MQTT-Reconnect, ein validiertes
  Statusmodell, Befehlsbestätigung und -timeouts, Screen-Power-Policy,
  serielles Diagnose-Dashboard sowie optionales, standardmäßig deaktiviertes
  OTA.
- `PoolStatusUpdater` trennt die nativ testbare Statuslogik von Arduino,
  Netzwerk und Broker; Tests decken MQTT-Randfälle und Broker-Integration ab.
- Dashboard-Konzept, Architektur, MQTT-Vertrag, Simulator, CI und
  LoxBerry-/Loxone-Anbindung sind dokumentiert.
- Uncommittet wurden `CODEX_HANDOFF.md` angelegt und der verbindliche
  Übergabeprozess in `AGENTS.md` definiert.

## Offene Arbeit

- Die beiden unversionierten Übergabedateien prüfen und erst nach ausdrücklicher
  Freigabe versionieren.
- Das Waveshare-Panel mit dem isolierten Bring-up gegen den Hersteller-Demoaufbau
  prüfen.
- ST7701-Display, GT911-Touch und Backlight in die normale Firmware portieren
  und auf echter Hardware testen.
- Anschließend LVGL aktivieren und die noch ausstehenden Dashboard-, Wartungs-
  und Einstellungsansichten vervollständigen.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller; das Panel zeigt bestätigte
  MQTT-Statuswerte und sendet nur Benutzeranforderungen.
- Keine optimistischen UI-Updates: Befehle gelten erst nach passender
  Statusmeldung als bestätigt. Fehlende oder alte Werte bleiben unbekannt bzw.
  stale; offline sind Bedienelemente deaktiviert.
- Hardwareunabhängige Poollogik bleibt vom Hardware- und Netzwerkcode getrennt
  und wird im nativen PlatformIO-Ziel getestet.
- Display-Support bleibt im Standard-Build deaktiviert, bis ST7701 und GT911 auf
  echter Hardware verifiziert sind; dafür existiert ein separates Bring-up-Ziel.
- Private Gerätewerte liegen nur in der ignorierten Datei
  `include/PoolConfig.h`; OTA ist standardmäßig deaktiviert.
- `CODEX_HANDOFF.md` ist vor jeder Sitzung zu lesen und am Sitzungsende gemäß
  `AGENTS.md` zu aktualisieren.

## Relevante Dateien

- `AGENTS.md`, `CODEX_HANDOFF.md` – verbindlicher Übergabeprozess und aktueller
  Arbeitsstand
- `README.md`, `docs/architecture.md` – Projektstatus und Architektur
- `platformio.ini` – Firmware-, Native-Test- und Display-Bring-up-Ziele
- `docs/ui.md`, `docs/mqtt.md`, `docs/loxone.md` – UI- und Integrationsvertrag
- `lib/Pool/`, `lib/Mqtt/MqttManager.*` – Zustandslogik und MQTT-Anbindung
- `src/main.cpp`, `src/display_bringup.cpp` – Firmware und Hardware-Smoke-Test
- `test/test_pool_state/test_main.cpp` – native Tests

## Ausgeführte Tests

- Git-Status sowie Tracking- und Staging-Diff geprüft: keine Änderungen an
  versionierten oder gestagten Dateien; nur zwei unversionierte Dateien.
- `git diff --no-index --check` für `CODEX_HANDOFF.md`: bestanden.
- Firmware-, Native- und MQTT-Integrationstests wurden nicht ausgeführt, da
  ausschließlich die Übergabedokumentation aktualisiert wird.

## Nächster konkreter Schritt

`AGENTS.md` und `CODEX_HANDOFF.md` inhaltlich prüfen und sie erst in einer
später ausdrücklich freigegebenen Aktion gemeinsam versionieren.
