# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für die über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Display, Backlight,
GT911-Touch und Screen-Off/Wake sind in der normalen Firmware integriert und
auf Hardware bestätigt. Als Nächstes folgen LVGL-Display-Flush und
LVGL-Touch-Input; MQTT-Bedienbefehle bleiben zunächst deaktiviert.

## Aktueller Git-Stand

- Branch `main`, Ausgangscommit `80ea39f` (`Hardware: Integrate display and
  GT911 into main firmware`).
- Arbeitsbaum enthält die noch nicht committete Screen-Power-Integration und
  zugehörige Dokumentation.
- Die normale Produktionsfirmware `esp32-s3-panel` ist auf `COM3` geflasht.
  Das isolierte Power-Testziel befindet sich nicht mehr auf dem Gerät.

## Erledigte Änderungen

- `AppController` bindet `ScreenPowerPolicy` an `DisplayManager` und konsumiert
  neue GT911-Press-Ereignisse genau einmal.
- Der erste Touch aus Off weckt nur und wird nicht an Bedienelemente
  weitergegeben. Bei wachem Display bleibt die GUI derzeit weiterhin aus.
- `DisplayManager` setzt Backlight-Endpunkte ohne PWM: GPIO 4 HIGH = aus,
  LOW = voll an. Gedimmte Zwischenwerte wären weiterhin per LEDC möglich.
- Vor Off wird der RGB-Framebuffer schwarz gefüllt. Beim Wake-up wird das
  Diagnosebild bei noch ausgeschaltetem Backlight wiederhergestellt und erst
  danach beleuchtet; dadurch gibt es kein altes, flackerndes Restbild.
- PWM-Dimmen ist für das Waveshare-4B-Panel deaktiviert. 2 %, 5 % und 10 % bei
  5 kHz sowie 10 % bei 20 kHz und 100 kHz flackerten sichtbar. Das Panel bleibt
  daher stabil bei 100 % und wechselt nach fünf Minuten direkt schwarz/aus.
- Die hardwareunabhängige `ScreenPowerPolicy` behält optionales Dimmen über den
  Konstruktor; bestehende Tests und andere Hardware können es weiter nutzen.
- Die Power-Zeiten sind per Build-Makro konfigurierbar. Das nicht standardmäßig
  ausgewählte Ziel `esp32-s3-power-test` verwendet 5/30 Sekunden für schnelle
  Hardwaretests, das normale Ziel weiterhin 60 Sekunden/5 Minuten. Da Dimmen
  am Panel deaktiviert ist, ist im normalen Betrieb nur die Off-Schwelle aktiv.
- `docs/ui.md` und `docs/architecture.md` beschreiben die bestätigte
  flackerfreie Strategie und den aktuellen LVGL-Stand.

## Offene Arbeit

- LVGL 8.4 initialisieren und den Arduino_GFX-Framebuffer als LVGL-Displayziel
  anbinden; zuerst nur den vorhandenen Hauptbildschirm anzeigen.
- GT911 als LVGL-Pointer registrieren. Der erste Touch aus Off muss weiterhin
  ausschließlich wecken; erst der nächste Touch darf LVGL erreichen.
- Danach den Hauptbildschirm auf echter Hardware auf Orientierung, Farben,
  Aktualisierung und Touch-Zuordnung prüfen.
- Erst anschließend MQTT-Bedienbefehle aktivieren und End-to-End gegen
  LoxBerry/Loxone prüfen: retained Startzustand, stale/offline,
  Befehlsbestätigung, Timeout und Reconnect.
- Native Tests erneut ausführen, sobald auf dem Windows-Host `gcc/g++` oder
  eine passende Native-Toolchain verfügbar ist.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel zeigt bestätigte MQTT-Werte;
  keine optimistischen UI-Updates.
- Statusmeldungen sind retained, Befehlstopics nicht. Unbekannte, stale oder
  offline Daten sperren die betroffenen Bedienelemente.
- Off bedeutet: Framebuffer schwarz, Backlight statisch aus, RGB-Panel und
  GT911 bleiben initialisiert. So wacht das Gerät ohne Panel-Neustart auf.
- PWM-Dimmen wird auf diesem konkreten Panel wegen nachgewiesenem Flackern nicht
  eingesetzt. Stabilität hat Vorrang vor einer Zwischenhelligkeit.
- Die unveränderten GT911-Achsen wurden per Vier-Ecken-Test bestätigt. Ein
  150-ms-Inaktivitätsfallback erkennt Touch-Release.
- Arduino_GFX ruft intern erneut `Wire.begin()` auf. Die Core-3.2-Warnung ist
  erwartet; die vorherige Initialisierung mit GPIO 47/48 bleibt notwendig.
- Private Gerätewerte liegen nur in der ignorierten `include/PoolConfig.h`.

## Relevante Dateien

- `AGENTS.md`, `CODEX_HANDOFF.md` – Übergabeprozess und aktueller Stand
- `platformio.ini` – normales Ziel, Power-Test und isolierte Bring-ups
- `lib/Pool/ScreenPowerPolicy.h` – Zeitlogik und optionales Dimmen
- `lib/Display/DisplayManager.*` – ST7701, Backlight, Blackout und GT911
- `lib/Core/AppController.*` – Screen-Power- und Wake-up-Integration
- `lib/Gui/`, `docs/ui.md` – nächste LVGL-Integrationsstufe

## Tatsächlich ausgeführte Prüfungen

- Screen-Power-Firmware mehrfach erfolgreich für das normale Ziel gebaut und
  auf `COM3` geflasht; sämtliche Images bestanden die Hash-Prüfung.
- ST7701, GT911 `911` an `0x5D` mit 480×480, Wi-Fi und MQTT liefen während der
  Power-Tests weiter.
- 60-Sekunden-/5-Minuten-Policyübergänge wurden seriell bestätigt. Der erste
  Touch aus Dimmed/Off meldete `wake touch; control forwarding suppressed` und
  weckte ohne Display-Neustart.
- Reines Backlight-Off ließ das weiße Bild flackernd sichtbar. Nach zusätzlichem
  `framebuffer blanked` bestätigte der Benutzer einen vollständig dunklen
  Off-Zustand und korrektes, flackerfreies Touch-Wake-up.
- PWM-Dimmversuche: 2 % flackerte deutlich, 5 % weniger, 10 % bei 5 kHz leicht,
  20 kHz leicht hochfrequent und 100 kHz stärker/unregelmäßig. Der abschließende
  Test ohne Dimm-PWM war bis zum direkten schwarzen Off flackerfrei.
- Das isolierte 5-/30-Sekunden-Ziel ohne Dimmen meldete
  `framebuffer blanked` und `power=off brightness=0%`; der Benutzer bestätigte
  flackerfreies Off und Wake-up.
- Das normale Ziel wurde danach neu gebaut: 51.932 Byte RAM (15,8 %) und
  1.033.310 Byte Flash (15,8 %), erfolgreich auf `COM3` geladen und gestartet.
  Der Boot-Log bestätigte `touch=ready`, IP `192.168.178.120` und MQTT verbunden.
- `git diff --check` wurde während der Änderungen wiederholt erfolgreich
  ausgeführt.
- Native Tests wurden nicht erneut ausgeführt; der Host besitzt weiterhin kein
  `gcc/g++`.

## Nächster konkreter Schritt

LVGL-Display-Flush in `DisplayManager` integrieren und zunächst den vorhandenen
Hauptbildschirm ohne Touch-Weitergabe oder MQTT-Befehle auf echter Hardware
anzeigen. Danach GT911 schrittweise als LVGL-Pointer anbinden.
