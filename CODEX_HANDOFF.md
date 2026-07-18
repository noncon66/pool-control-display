# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für die über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. LVGL-Ausgabe, GT911-Pointer,
Backlight und sichere Screen-Off/Wake-Logik sind integriert und auf Hardware
bestätigt. Als Nächstes können produktive MQTT-Bedienbefehle bewusst aktiviert
und End-to-End gegen LoxBerry/Loxone geprüft werden.

## Aktueller Git-Stand

- Branch `main`, Ausgangscommit `90d4287` (`Display: Add stable screen-off and
  touch-wake behavior`).
- Uncommittete Änderungen in `DisplayManager`, `AppController`, `GuiManager`,
  `WifiManager` und `CODEX_HANDOFF.md`.
- Die normale Fünf-Minuten-Firmware `esp32-s3-panel` ist auf `COM3` geflasht;
  das verkürzte Power-Testziel befindet sich nicht mehr auf dem Gerät.

## Erledigte Änderungen

- `DisplayManager` registriert das 480×480-ST7701-Panel über Arduino_GFX und
  einen LVGL-Flush-Callback. Der partielle Puffer umfasst 40 Zeilen/38.400 Byte
  im PSRAM.
- GT911 ist als LVGL-Pointer registriert und liefert Koordinaten sowie
  Press/Release. Der LVGL-Timer pausiert während Screen-Off.
- Ein Wake-Touch bleibt bis zum vollständigen Loslassen für LVGL gesperrt.
  Erst ein neuer Touch bei bereits wachem Display darf das UI erreichen.
- `GuiManager` hat einen separaten `controlsEnabled`-Schalter. Im sicheren
  Bring-up ist er `false`: Buttons reagieren visuell und protokollieren den
  Treffer, aber alle MQTT-Befehle sind hart gesperrt.
- Im Touch-Testmodus bleiben Buttons trotz unbekannter/stale Daten visuell
  testbar. Bei später aktivierter Produktivbedienung greift automatisch wieder
  die normale Offline-/Stale-/Unknown-Sperre.
- `WifiManager` setzt `WiFi.persistent(false)`, damit Reconnects nicht unnötig
  Flash schreiben und dabei mit der kontinuierlichen RGB-DMA kollidieren.
  Der Reconnect-Timer zählt nun ab dem tatsächlichen Verbindungsversuch.
- PWM-Dimmen bleibt wegen nachgewiesenem Flackern deaktiviert. Das Panel bleibt
  bei 100 % und wechselt nach fünf Minuten direkt schwarz/aus.

## Offene Arbeit

- Produktive MQTT-Bedienung bewusst aktivieren (`controlsEnabled=true`) und
  dabei die bestehende Offline-/Stale-/Unknown-Freigabe beibehalten.
- End-to-End gegen LoxBerry/Loxone prüfen: retained Startzustand,
  Betriebsmodus, Solltemperatur, Filterpumpe, Befehlsbestätigung, Timeout,
  stale/offline und Reconnect.
- Die im UI derzeit unbekannten Loxone-Daten beziehungsweise retained Topics
  prüfen, bevor echte Befehle ausgelöst werden.
- Native Tests erneut ausführen, sobald auf dem Windows-Host `gcc/g++` oder
  eine passende Native-Toolchain verfügbar ist.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel zeigt bestätigte MQTT-Werte;
  keine optimistischen UI-Updates.
- Statusmeldungen sind retained, Befehlstopics nicht. Unbekannte, stale oder
  offline Daten sperren die betroffenen Bedienelemente.
- Off bedeutet: Framebuffer schwarz, Backlight statisch aus, RGB-Panel und
  GT911 bleiben initialisiert.
- Keine RGB-DMA-Bounce-Buffer: Ein Versuch mit 20 Zeilen verschob den
  Cache/PSRAM-Absturz nur in `lcd_rgb_panel_fill_bounce_buffer` und wurde
  vollständig zurückgenommen.
- WLAN-Konfiguration bleibt zur Laufzeit nichtpersistent, um Flash-Cache-Stalls
  während aktiver RGB-DMA zu vermeiden.
- Die unveränderten GT911-Achsen wurden per Vier-Ecken- und Button-Test
  bestätigt. Ein 150-ms-Inaktivitätsfallback erkennt Touch-Release.
- Private Gerätewerte liegen nur in der ignorierten `include/PoolConfig.h`.

## Relevante Dateien

- `AGENTS.md`, `CODEX_HANDOFF.md` – Übergabeprozess und aktueller Stand
- `platformio.ini` – normales Ziel, Power-Test und isolierte Bring-ups
- `lib/Display/DisplayManager.*` – ST7701, LVGL-Flush/Pointer, Backlight, GT911
- `lib/Core/AppController.*` – GUI-, Screen-Power- und Wake-Integration
- `lib/Gui/GuiManager.*`, `lib/Gui/PanelViewModel.*` – Hauptansicht und sichere
  Bedienfreigabe
- `lib/PoolNetwork/WifiManager.*` – nichtpersistente WLAN-Reconnects
- `lib/Pool/ScreenPowerPolicy.h` – Zeitlogik und optionales Dimmen

## Tatsächlich ausgeführte Prüfungen

- LVGL-Hauptansicht auf Hardware visuell bestätigt: Orientierung, Farben,
  Layout und Darstellung sehen gut aus.
- Touch-Koordinaten bestätigt: `AUTOMATIK` etwa (250, 310), Minus (70, 425),
  Plus (445, 420). Alle Buttons reagieren nach Testmodus-Korrektur sichtbar.
- Serielle Callback-Prüfung bestätigt `mode touch=1`, `target touch=minus` und
  `target touch=plus`, jeweils mit `MQTT control suppressed`.
- Während eines WLAN-Reconnects trat zunächst `Cache disabled but cached memory
  region accessed` im RGB-DMA-ISR auf. Backtrace mit `addr2line` dekodiert.
- Ein Bounce-Buffer-Versuch schlug fehl und wurde zurückgenommen. Mit
  `WiFi.persistent(false)` lief der folgende 45-Sekunden-Test inklusive WLAN,
  MQTT und zahlreichen Touches ohne Panic.
- Der danach korrigierte erste Reconnect-Versuch startete ohne unnötigen
  zweiten `WiFi.begin`, ohne Panic und verband WLAN/MQTT erfolgreich.
- Verkürztes Power-Testziel erfolgreich gebaut und geflasht. Seriell bestätigt:
  Off nach 30 Sekunden, erster Touch nur `wake touch; control forwarding
  suppressed`, zweiter Touch `mode touch=1; MQTT control suppressed`.
- Normale Firmware abschließend gebaut: 102.532 Byte RAM (31,3 %) und
  1.247.334 Byte Flash (19,0 %), erfolgreich mit Hash-Prüfung auf COM3 geladen.
- `git diff --check` war vor den Builds erfolgreich. Temporäre Capture-Skripte
  wurden entfernt.
- Native Tests nicht ausgeführt; dem Host fehlt weiterhin `gcc/g++`.

## Nächster konkreter Schritt

Zuerst klären, warum keine retained Loxone-Zustände ankommen. Sobald gültige
Werte vorliegen, `controlsEnabled=true` setzen und die drei Befehlswege einzeln
End-to-End testen, beginnend mit einem risikoarmen Betriebsmodus-Test.
