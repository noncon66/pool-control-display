# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für die über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Display, Backlight und
GT911-Touch sind jetzt in der normalen Firmware integriert und gemeinsam auf
Hardware bestätigt. Als nächste Stufe folgt die Anbindung der vorhandenen
`ScreenPowerPolicy`; LVGL und Bedienbefehle bleiben bis dahin deaktiviert.

## Aktueller Git-Stand

- Branch `main`, Ausgangscommit `0bfa87a`; Arbeitsbaum enthält die noch nicht
  committete normale Hardwareintegration.
- Geändert sind `platformio.ini`, `lib/Core/` und `lib/Display/`.
- `lib/Network/` wurde wegen der Namenskollision mit Arduino Core 3.2.0 nach
  `lib/PoolNetwork/` verschoben; öffentliche Header- und Klassennamen blieben
  unverändert.

## Erledigte Änderungen

- Der normale Build verwendet jetzt pioarduino `54.03.20` mit Arduino-ESP32
  Core 3.2.0 und aktiviert USB CDC beim Boot.
- Arduino_GFX 1.6.0 ist eine Abhängigkeit des normalen Ziels; die frühere
  Ausklammerung von `lib/Display` wurde entfernt.
- `DisplayManager` verwendet die am echten Waveshare
  ESP32-S3-Touch-LCD-4B bestätigten ST7701-/RGB-Pins und Timings, TCA9554,
  I2C GPIO 47/48, GT911 an `0x5D`/`0x14` und aktives-low Backlight-PWM an
  GPIO 4.
- Beim Start erscheint ein weißes Hardware-Diagnosebild mit rotem
  `Pool Control` und schwarzem Status für Display, Touch und deaktiviertes
  LVGL. Raw-Touch-Ereignisse werden seriell ausgegeben.
- `AppController` startet und pollt `DisplayManager` vor bzw. neben den
  bestehenden Wi-Fi-, MQTT- und OTA-Komponenten. `GuiManager` wird weiterhin
  nicht initialisiert; Touch löst keine Steuerbefehle aus.
- Die lokale Bibliothek heißt nun `PoolNetwork`, damit sie nicht mit der neuen
  Framework-Bibliothek `Networking` kollidiert.
- Weil die WiFi-Metadaten des Arduino Core 3.2.0 ihre Abhängigkeit zu
  `Networking` nicht deklarieren, ergänzt das normale Ziel dessen Include-Pfad
  portabel über PlatformIOs `$PROJECT_PACKAGES_DIR`.

## Offene Arbeit

- `ScreenPowerPolicy` mit Backlight und Touch-Wakeup verbinden; Schlafen und
  Aufwachen ohne Display-Neustart auf Hardware prüfen.
- LVGL 8.4 aktivieren, Display-Flush und Touch-Input registrieren und den
  vorhandenen Hauptbildschirm auf echter Hardware testen.
- Danach MQTT/LoxBerry-End-to-End prüfen: retained Startzustand, stale/offline,
  Befehlsbestätigung, Timeout und Reconnect.
- Native Tests erneut ausführen, sobald auf dem Windows-Host `gcc/g++` oder
  eine passende Native-Toolchain verfügbar ist.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel zeigt bestätigte MQTT-Werte;
  es gibt keine optimistischen UI-Updates.
- Statusmeldungen sind retained, Befehlstopics nicht. Unbekannte, stale oder
  offline Daten sperren die betroffenen Bedienelemente.
- Hardwareintegration erfolgt weiterhin stufenweise: derzeit sind Display,
  Backlight und reine Touch-Diagnose aktiv, LVGL und Bedienung bewusst aus.
- Das offizielle Waveshare-BSP und die erfolgreichen isolierten Bring-ups sind
  die Quelle für Pinbelegung, Timing, Resetfolge und Orientierung.
- Die unveränderten GT911-Achsen wurden am realen Panel per Vier-Ecken-Test
  bestätigt. Ein 150-ms-Inaktivitätsfallback erkennt Touch-Release.
- Arduino_GFX ruft intern erneut `Wire.begin()` auf. Die daraus entstehende
  Core-3.2-Warnung ist erwartet; die vorherige Initialisierung mit GPIO 47/48
  bleibt notwendig.
- Private Gerätewerte liegen nur in der ignorierten `include/PoolConfig.h`.

## Relevante Dateien

- `AGENTS.md`, `CODEX_HANDOFF.md` – Übergabeprozess und aktueller Stand
- `platformio.ini` – Standard-, Native-Test- und isolierte Bring-up-Ziele
- `lib/Display/DisplayManager.*` – ST7701, TCA9554, Backlight und GT911
- `lib/Core/AppController.*` – Integration in die normale Firmware
- `lib/PoolNetwork/` – Wi-Fi und OTA nach der Framework-Namenskollision
- `lib/Pool/ScreenPowerPolicy.h`, `lib/Gui/` – nächste Integrationsstufen
- `src/display_bringup.cpp`, `src/touch_bringup.cpp`,
  `tools/display_bringup.py` – weiterhin verfügbare isolierte Diagnosen

## Tatsächlich ausgeführte Prüfungen

- Mehrere normale Builds dienten der Migration; die ersten scheiterten an
  fehlendem `Network.h`. Nach Umbenennung zu `PoolNetwork` und portablem
  Framework-Include war `pio run -e esp32-s3-panel` erfolgreich: 51.916 Byte
  RAM (15,8 %) und 1.031.694 Byte Flash (15,7 %).
- `pio test -e native` wurde gestartet, konnte aber nicht gebaut werden, weil
  auf dem Host weder `gcc` noch `g++` gefunden wurde. Es wurden daher keine
  nativen Testfälle ausgeführt.
- Die normale Firmware wurde erfolgreich auf den ESP32-S3 an `COM3` geladen;
  Chip, 8 MB PSRAM und 16 MB Flash wurden erkannt, alle Images bestanden die
  Hash-Prüfung.
- Ein kontrollierter serieller RTS-Reset zeigte:
  `ST7701 initialized`, GT911 Produkt `911` an `0x5D`, Konfiguration 79,
  Auflösung `480x480`, `touch=ready, LVGL=disabled`.
- Im selben Boot verband sich das Gerät mit Wi-Fi (`192.168.178.120`) und dem
  MQTT-Broker; Abonnements wurden erfolgreich eingerichtet.
- Der Benutzer bestätigte in der normalen Firmware den weißen Hintergrund,
  das rote `Pool Control` sowie die schwarzen Meldungen
  `Hardware integration OK`, `GT911 touch ready` und `LVGL disabled`.
- Ein kurzer Tipp auf das laufende normale System wurde seriell als
  `[Display] touch raw x=269 y=309 strength=18` erfasst.
- Die früheren isolierten Display- und Touch-Bring-ups bleiben auf realer
  Hardware bestätigt: sichtbares Testbild nach zwei Power-Cycles sowie
  bestandener GT911-Vier-Ecken-Test ohne Achsentausch oder Spiegelung.

## Nächster konkreter Schritt

`ScreenPowerPolicy` an Backlight und Touch-Wakeup anbinden und anschließend
Schlafen sowie Aufwachen ohne Display-Neustart auf echter Hardware prüfen.
