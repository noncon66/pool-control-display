# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für eine über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Nach erfolgreichem isoliertem
Display-Smoke-Test folgt die schrittweise Hardwareintegration: zuerst
GT911-Touch und Koordinaten, dann Display/Touch in der normalen Firmware,
Screen-Power-Policy und zuletzt die vorhandene LVGL-Oberfläche.

## Aktueller Git-Stand

- Branch `main`, identisch mit `origin/main`; der Arbeitsbaum enthält die drei
  unten genannten ungestagten Änderungen aus dem Display-Bring-up.
- Aktueller Commit: `67ada5e` (`Docs: Handoff update`).
- Durch diese Sitzung sind `CODEX_HANDOFF.md`, `platformio.ini` und
  `tools/display_bringup.py` ungestaged geändert.

## Bereits erledigt

- Wi-Fi-/MQTT-Reconnect, validiertes Statusmodell, strikte Payload-Auswertung,
  Befehlsbestätigung und -timeouts, Screen-Power-Policy, serielles
  Diagnose-Dashboard sowie optionales, standardmäßig deaktiviertes OTA sind
  implementiert.
- `PoolStatusUpdater` trennt Topic-/Payload-Verarbeitung von Arduino, Netzwerk
  und Broker. Die native Testsuite enthält 31 registrierte Unity-Tests.
- Simulator und CI prüfen Firmware-Build, native Logik sowie MQTT-Status-,
  Befehls- und Retain-Vertrag gegen einen isolierten Mosquitto-Broker.
- Die LoxBerry-/Loxone-Architektur ist dokumentiert: Befehle über HTTP Virtual
  Inputs, bestätigte Zustände über UDP-Port `11884` als retained MQTT-Werte.
- Das Panel wurde als `ESP32-S3-Touch-LCD-4B Rev2.2` identifiziert. Pinbelegung,
  TCA9554, Resetfolge und Backlight des isolierten Display-Bring-ups stimmen
  mit aktuellem Waveshare-Schaltbild und BSP überein.
- Das isolierte Ziel `esp32-s3-display-bringup` zeigt nur ein statisches
  Testbild; Touch, LVGL, Wi-Fi, MQTT und Poolsteuerung bleiben dort bewusst aus.
- Das isolierte Bring-up wurde auf das Gerät an `COM3` geflasht. Drei
  aufeinanderfolgende RTS-Resets booteten sauber und meldeten jeweils
  `[Bring-up] display initialized`; Build und serieller Teil des Smoke-Tests
  sind damit bestätigt.
- Der Display-Smoke-Test ist vollständig bestanden: Am Gerät wurden weißer
  Hintergrund, rote Überschrift `Pool Control` und schwarzer Text
  `Display bring-up OK` bestätigt. Zwei vollständige Strom-Aus-/Ein-Zyklen
  zeigten das Testbild erneut stabil.

## Offene Arbeit

- Einen isolierten GT911-Test aus dem offiziellen Waveshare-BSP portieren:
  I2C-Erkennung, Rohkoordinaten, Orientierung, Achsentausch und Randpunkte prüfen.
- ST7701-, GT911- und Backlight-Treiber in die normale Firmware portieren und
  dabei den Wechsel vom Arduino-Core 2.0.17 auf den im Bring-up verwendeten
  Core 3.2.0 kontrolliert durchführen.
- `ScreenPowerPolicy` an Backlight und Touch-Wakeup anbinden; Schlafen und
  Aufwachen ohne Display-Neustart prüfen.
- LVGL 8.4 aktivieren, Display-Flush und Touch-Input registrieren und zuerst
  den vorhandenen Hauptbildschirm auf echter Hardware testen.
- Danach MQTT/LoxBerry-End-to-End prüfen: retained Startzustand, stale/offline,
  Befehlsbestätigung, Timeout und Reconnect. Erst anschließend Wartungs- und
  Einstellungsansichten ergänzen.
- Die reale LoxBerry-/Loxone-Zuordnung einrichten: drei virtuelle
  Befehlseingänge, UDP-Ausgang `11884`, sieben retained Statustopics und
  Aktualisierung spätestens alle 30 Sekunden.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel zeigt bestätigte MQTT-Werte;
  keine optimistischen UI-Updates.
- Statusmeldungen sind retained, Befehlstopics nicht. Unbekannte, stale oder
  offline Daten sperren die betroffenen Bedienelemente.
- Displayintegration bleibt bis zur Hardwareverifikation vom Standard-Build
  getrennt. Änderungen werden in kleinen, rückrollbaren Stufen integriert.
- Das offizielle Waveshare-BSP ist die Quelle für ST7701, GT911, Timing,
  IO-Expander, Reset und Backlight. Der Aufdruck Rev2.2 wird nicht ungeprüft
  mit der BSP-Bezeichnung V1.0 gleichgesetzt.
- Private Gerätewerte liegen nur in der ignorierten `include/PoolConfig.h`.
- Python bleibt das einzige Frontend für Simulator und Display-Bring-up.
- Das Bring-up-Ziel aktiviert `ARDUINO_USB_CDC_ON_BOOT=1`, damit Arduino
  `Serial` über den USB-Serial/JTAG-Port ausgegeben wird. Der Python-Launcher
  wählt auch beim Monitor explizit das isolierte Bring-up-Environment.

## Relevante Dateien

- `AGENTS.md`, `CODEX_HANDOFF.md` – Übergabeprozess und aktueller Stand
- `docs/hardware.md`, `src/display_bringup.cpp`, `tools/display_bringup.py` –
  Hardware-Bring-up
- `platformio.ini` – Standard-, Native-Test- und isoliertes Bring-up-Ziel
- `docs/ui.md`, `lib/Display/`, `lib/Pool/ScreenPowerPolicy.h` – UI und
  Displayintegration
- `docs/mqtt.md`, `docs/loxone.md`, `tools/loxone_mqtt_simulator.py` – Vertrag
  und reale Anbindung
- `src/main.cpp`, `lib/Pool/`, `lib/Mqtt/` – normale Firmware
- `test/test_pool_state/test_main.cpp` – 31 native Unity-Tests

## Tatsächlich ausgeführte Prüfungen

- `python tools/display_bringup.py build` über PlatformIO-Python wurde für
  `esp32-s3-display-bringup` erfolgreich ausgeführt: Arduino Core 3.2.0,
  22.684 Byte RAM (6,9 %) und 398.322 Byte Flash (6,1 %).
- `python tools/display_bringup.py upload --port COM3` wurde erfolgreich
  ausgeführt. Esptool erkannte ESP32-S3 Revision 0.2, 8 MB PSRAM und 16 MB
  Flash; alle geschriebenen Images bestanden die Hash-Prüfung.
- Ein kontrollierter serieller RTS-Reset und zwei Wiederholungen wurden bei
  115200 Baud ausgeführt. Alle drei Bootvorgänge meldeten den Boardnamen und
  `[Bring-up] display initialized` ohne Initialisierungsfehler.
- Der Monitorpfad des Python-Launchers wurde ausgeführt. Der zunächst
  aufgetretene `UnknownPlatform`-Fehler wurde durch explizite Wahl des
  Bring-up-Environments behoben; der Monitor öffnete danach `COM3`.
- Der Benutzer bestätigte am realen Gerät die korrekte sichtbare Darstellung
  und zwei erfolgreiche vollständige Strom-Aus-/Ein-Zyklen.
- Native-, Broker- und MQTT-Tests wurden in dieser Sitzung nicht ausgeführt.

## Nächster konkreter Schritt

Einen isolierten GT911-Touch-Test auf Basis des offiziellen Waveshare-BSP
implementieren. Er soll serielle Rohkoordinaten ausgeben und Orientierung,
Achsentausch sowie alle vier Displayränder prüfen, ohne bereits LVGL, WLAN,
MQTT oder Poolbefehle zu aktivieren.
