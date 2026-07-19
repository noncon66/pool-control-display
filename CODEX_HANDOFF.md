# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für die über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Der Modusvertrag und der
Befehlsweg sind korrigiert und bestätigt. Als Nächstes müssen die Loxone-
Statuswerte regelmäßig retained publiziert werden, damit das Panel nicht nach
60 Sekunden in den sicheren Stale-Zustand wechselt.

## Aktueller Git-Stand

- Branch `main`, aktueller Commit `9396b3b` (`MQTT: Align pool mode values with
  Loxone`).
- Vor dieser Handoff-Aktualisierung war der Arbeitsbaum sauber.
- Die korrigierte Firmware ist auf `COM3` geflasht.
- Kein Simulator- oder Hilfsprozess läuft.

## Erledigte Änderungen

- Verbindlicher Modusvertrag: `1 = Automatik`, `2 = Manuell`, `3 = Aus`; `0`
  wird abgelehnt.
- Enum, Parser, MQTT-Validierung, UI-Buttonwerte/-Hervorhebung, Simulator,
  Tests und Dokumentation verwenden dieselbe Zuordnung.
- Firmware erfolgreich gebaut, geflasht und auf dem Display geprüft.
- Display/Testclient → Mosquitto → LoxBerry Gateway → virtueller Loxone-Eingang
  funktioniert end-to-end.
- Für `pool/cmd/mode` ist im Gateway `Disable Cache` aktiv und
  `Reset-After-Send` aus. Ein wiederholter identischer Wert wurde laut
  Gateway-Log erneut per HTTP an Loxone gesendet.
- `pool/status/mode = 1` wurde retained vom Broker gelesen und vom Panel als
  `Mode : Auto` ausgewertet. `AUS` bei Status `3` war zuvor ebenfalls visuell
  bestätigt.
- Alle drei Displaybuttons sind end-to-end bestätigt: jeweils genau ein Touch
  und ein nicht-retained Publish, danach Loxone-Status und Displaywechsel:
  Automatik `1 → Auto`, Manuell `2 → Manual`, Aus `3 → Off`. Abschließend steht
  die Anlage wieder auf Automatik.

## Offene Arbeit

- Loxone muss `pool/status/mode` spätestens alle 30 Sekunden retained erneut
  publizieren; derzeit wird der Wert nach 60 Sekunden am Panel stale.
- Die übrigen sechs tatsächlichen Zustände retained publizieren:
  `waterTemp`, `targetTemp`, `filterPump`, `heatingPump`, `heatingAllowed` und
  `isHeating` unter `pool/status/`.
- Anschließend Rückbestätigung, Timeout, stale/offline und Reconnect testen.
- Native Tests erneut ausführen, sobald `gcc/g++` auf dem Host verfügbar ist.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel übernimmt Commands erst nach
  einer bestätigenden Statusmeldung.
- Statusmeldungen sind retained, Befehlstopics nicht.
- Ohne frische Statusdaten bleiben produktive Controls gesperrt.
- `Disable Cache` stellt bei Modusbefehlen auch die Weiterleitung identischer
  Werte sicher, ohne einen ungültigen Resetwert `0` zu erzeugen.
- Private Gerätewerte liegen nur in der ignorierten `include/PoolConfig.h`.

## Relevante Dateien

- `lib/Pool/PoolState.h`, `lib/Pool/MqttPayloadParser.h` – Modusvertrag/Parser
- `lib/Gui/GuiManager.cpp` – Buttonwerte und Statushervorhebung
- `lib/Mqtt/MqttManager.cpp` – Command-Validierung und Publish
- `tools/loxone_mqtt_simulator.py` – Broker-Integrationstest
- `docs/mqtt.md`, `docs/loxone.md` – verbindliche Integration

## Tatsächlich ausgeführte Prüfungen

- Firmware-Releasebuild erfolgreich: RAM 31,3 %, Flash 19,0 %.
- Firmware auf `COM3` geladen; alle Flash-Hashes verifiziert.
- Retained Status `3` wurde als `Off` erkannt; Benutzer bestätigte `AUS` grün.
- Retained Status `1` wurde als `Auto` erkannt; seriell zunächst `CURRENT`, nach
  60 Sekunden ohne Wiederholung erwartungsgemäß `STALE`.
- Modusbefehl `1` wurde vom Panel korrekt nicht-retained publiziert.
- Direkte Werte `1`, `2` und wieder `1` erreichten den virtuellen Loxone-
  Eingang; ein wiederholtes `2` wurde mit `Disable Cache` erneut weitergeleitet.
- Broker-Subscriber bestätigte Topic, Payload und Retain-Flag der Tests.
- Hardware-End-to-End-Test aller Modusbuttons erfolgreich: `2`, `3`, `1`
  jeweils einmal publiziert, jeweils sofort über retained Status bestätigt und
  korrekt als Manual, Off, Auto angezeigt; Datenstatus blieb `CURRENT`.
- Erster 75-Sekunden-Monitorlauf sah nur retained `mode = 1`, wurde aber nach
  rund 45 Sekunden wegen fehlendem Testclient-Keepalive getrennt.
- Wiederholter 75-Sekunden-Monitorlauf mit aktivem Keepalive blieb stabil und
  sah ebenfalls ausschließlich den retained Startwert `1`; keine zyklische
  Live-Meldung kam an. Der neue Loxone-Wiederholzweig sendet noch nicht.
- Auf Benutzerwunsch wurde derselbe stabile 75-Sekunden-Test erneut ausgeführt;
  Ergebnis unverändert: nur retained `mode = 1` bei 0,1 Sekunden, danach keine
  Live-Meldung.
- Screenshot der Loxone-Konfiguration geprüft: bestehender analoger virtueller
  Ausgangsbefehl mit `retain pool/status/mode <v>`, `Erste Wiederholung = 30 s`,
  `Abstand Wiederholung = 30 s`, Digitalausgang deaktiviert. Diese eingebaute
  Wiederholung ersetzt den zuvor vorgeschlagenen separaten Taktgeber.
- Wahrscheinlich wurde die Wiederholkette nach dem Speichern noch nicht
  gestartet, weil der analoge Eingang ohne erneute Wertänderung auf `1` blieb.
- Mit Benutzerzustimmung wurde der Modus einmal `2 → 1` gewechselt. Danach war
  die eingebaute Wiederholung aktiv: Broker-Messung über 75 Sekunden sah den
  retained Startwert bei 0,1 s sowie Live-Publishes `mode = 1` bei 5,4 s,
  35,4 s und 65,4 s, jeweils exakt 30 Sekunden Abstand.
- Der analoge `<v>`-Ausgang wiederholt automatisch alle drei Moduswerte; die
  zuvor erwogenen getrennten Takt-/Moduszweige sind nicht erforderlich.
- Hardware-Langzeittest über 82 Sekunden bestanden: Nach seriellem Neustart
  empfing das Panel den retained Modus und drei weitere `mode = 1`-Meldungen;
  `Loxone data` blieb über die 60-Sekunden-Grenze hinaus `CURRENT`.
- Die bestätigte Loxone-Konfiguration wurde in `docs/loxone.md` ergänzt.
- `pool/status/targetTemp = 28.0` ist retained angebunden und wurde über
  75 Sekunden bestätigt: retained Startwert sowie Live-Wiederholungen nach
  jeweils 30 Sekunden.
- Zwei Hardwaretests der Plus-Taste erzeugten Touchdaten innerhalb des Buttons,
  aber kein LVGL-Klickereignis. Die Koordinaten wanderten beim Abheben bis an
  den unteren Buttonrand; deshalb wurden Plus/Minus auf `LV_EVENT_PRESSED`
  umgestellt.
- Firmware mit reduziert parallelem Build erfolgreich gebaut und auf `COM3`
  geflasht; Hashes verifiziert. Der erste Buildversuch überschritt das
  Zeitlimit und hinterließ kurz verwaiste Prozesse, die anschließend beendet
  beziehungsweise bereits ausgelaufen waren.
- Hardwaretest mit neuem Handler erfolgreich: Plus änderte den Sollwert von
  `28.0` auf `28.5`, Loxone meldete retained `28.5` zurück und das Display
  zeigte den bestätigten Wert. Benutzer meldete danach ein blasseres Display
  und weiterhin nicht angenommene Eingaben. Ein Wake-Touch half nicht.
- Codeprüfung: `ScreenPowerPolicy` läuft mit deaktiviertem Dimm-Zwischenzustand;
  die blassere Darstellung stammt daher wahrscheinlich von LVGLs Disabled-
  Zustand. Eine zustandsänderungsbasierte GUI-Diagnoseausgabe für die drei
  Controls und den Sollwert-Commandstatus wurde ergänzt, gebaut und geflasht.
- Diagnose bestätigte zunächst den korrekten Plus-Ablauf `28.5 → 29.0`:
  target enabled, kurz Pending/disabled, Statusbestätigung, danach Confirmed
  und wieder enabled.
- Beim späteren Minusversuch zeigte das Log jedoch
  `controls mode=disabled target=disabled`, obwohl Zieltemperaturmeldungen
  ankamen und der globale Datenstatus `CURRENT` war. Es kam keine zyklische
  Modus-Livemeldung mehr; der retained Modus war älter als 60 Sekunden und
  sperrte deshalb korrekt alle modusabhängigen Controls. Die blasse Darstellung
  ist LVGL Disabled, kein Backlight-/Touchfehler.
- Wahrscheinlicher Auslöser: Der Download der neuen TargetTemp-Loxone-
  Konfiguration stoppte die eingebaute Wiederholkette des analogen Modus-
  Ausgangs. Da der Eingang auf `1` blieb, startete sie nicht neu.
- Benutzer bewertet eine robuste zyklische Loxone-Triggerlogik als zu
  aufwändig und fragt nach einer Firmware-Alternative.
- Firmware auf retained/MQTT-Politik umgestellt: bekannte Werte bleiben bei
  MQTT-Verbindung bedienbar, individuelle 60-Sekunden-Altersgrenzen sind nur
  noch Diagnose, MQTT-Offline/unbekannte Werte sperren weiterhin, Command-
  Bestätigung/Timeout bleiben bestehen. Dokumentation und Tests angepasst.
- Firmware gebaut, geflasht und 78 Sekunden getestet: Controls blieben über
  die alte Stale-Grenze hinaus enabled.
- Der aktuell empfangene Sollwert war `20.1`. Stumpfes ±0,5 ergab ungültige
  Rasterwerte `20.6`/`19.6` und wurde deshalb abgelehnt. Plus/Minus runden nun
  richtungsabhängig auf den nächsten gültigen Halbgrad (`20.5`/`20.0`).
- Raster-Firmware erfolgreich gebaut und geflasht. Hardwaretest bestanden:
  Beim bestätigten Zwischenwert `28.2` erzeugte ein Plus-Tipp genau einen
  Command `28.5`; Loxone bestätigte `28.5`, Pending wurde aufgehoben und das
  Control wieder enabled.
- `pool/status/targetTemp` schwankte während des Tests ohne Panel-Touch zwischen
  `29.0`, `28.9`, `28.8`, `28.7`, `28.2` und nach Bestätigung noch `28.0`.
  Klären, ob der Benutzer parallel in Loxone änderte oder der Statusausgang
  nicht am stabilen tatsächlich aktiven Sollwert hängt.
- Native Tests nicht ausgeführt; dem Host fehlt `gcc/g++`.

## Nächster konkreter Schritt

Klären, warum `pool/status/targetTemp` während des Tests selbstständig schwankte.
Falls der Benutzer nicht parallel änderte, in Loxone die Quelle des virtuellen
Ausgangs auf den stabilen tatsächlich übernommenen Sollwert korrigieren. Danach
Plus und Minus jeweils einmal bestätigen; anschließend Filterpumpenstatus und
-bedienung anbinden.
