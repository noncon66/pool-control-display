# Codex-Handoff

## Aktuelles Ziel

Das ESP32-S3-Pooldisplay als dünnen MQTT-Client für die über LoxBerry
angebundene Loxone-Poolsteuerung fertigstellen. Modus, Sollwert und
Filterpumpenbedienung funktionieren end-to-end. Die Statuswerte für
Filterpumpe, Heizpumpe und Heizfreigabe sind angebunden. Der nicht benötigte
separate Status `isHeating` wurde aus dem Projekt entfernt.

## Aktueller Git-Stand

- Branch `main`, aktueller Commit `c7ddf71` (`Docs: Record integration status
  and GUI design direction`).
- Vor dieser Handoff-Aktualisierung war der Arbeitsbaum sauber.
- Die aktuelle Firmware ohne `isHeating` ist auf `COM3` geflasht.
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

- Heizpumpenstatus `0` bei der nächsten regulären Abschaltung kontrollieren.
- Ursache der früheren Schwankungen von `pool/status/targetTemp` prüfen und
  sicherstellen, dass Loxone den stabilen tatsächlich aktiven Sollwert sendet.
- GUI später überarbeiten, derzeit auf ausdrücklichen Benutzerwunsch noch
  nicht ändern: Plus-/Minus-Schaltflächen sind als blaue Flächen sichtbar,
  ihre Zeichen aber kaum erkennbar; mehrere Beschriftungen sind für den realen
  Betrachtungsabstand zu klein.
- Die aktuellen Gerätefotos `WhatsApp Image 2026-07-19 at 23.51.47.jpeg` und
  `WhatsApp Image 2026-07-19 at 23.40.55.jpeg` bestätigen die GUI-Probleme:
  Plus/Minus sind in den blauen Tasten praktisch unsichtbar, die Filterpumpen-
  Kachel ist nicht als Bedienelement erkennbar und die vielen gleichartigen
  grauen Karten/Rahmen wirken deutlich technischer als die Loxone-App.
- Weitere Screenshots der aktuellen Loxone-App sind angekündigt und sollen vor
  der nächsten GUI-Umsetzung als visuelle Referenz ausgewertet werden. Zu
  übernehmen sind Designprinzipien, nicht Loxone-Branding oder eine direkte
  Kopie der App-Screens.
- Verbindliche GUI-Vorgabe des Benutzers: Das aktuelle Display-Layout und die
  Anordnung der Bereiche bleiben erhalten. Die Filterpumpen-Kachel muss bei der
  späteren visuellen Überarbeitung eindeutig als bedienbarer Button erkennbar
  werden; derzeit wirkt sie wie die benachbarten reinen Statuskarten für
  Heizpumpe und Heizfreigabe. Noch keine Umsetzung gewünscht.
- Auf anschließenden Benutzerwunsch wurde ein interaktiver visueller Entwurf
  außerhalb des Firmwareprojekts erstellt. Er behält das bestehende Layout,
  vergrößert die Typografie und Plus-/Minus-Zeichen und kennzeichnet die
  Filterpumpen-Kachel durch Power-Symbol und sichtbaren Schalthinweis eindeutig
  als Button. Der Entwurf ändert lokal nur seinen simulierten Zustand.
- Auf Benutzerwunsch wurde zusätzlich eine konsequent dunkle Variante des
  interaktiven Entwurfs erstellt. Layout, größere Typografie, klar sichtbare
  Plus-/Minus-Zeichen und Button-Kennzeichnung der Filterpumpe bleiben gleich.
- Eine weitere dunkle Entwurfsvariante verwendet auf Benutzerwunsch deutlich
  mehr Loxone-inspiriertes Grün: aktiver Modus, Verbindungsanzeigen,
  Statuswerte, Filterpumpen-Affordance sowie Plus/Minus sind stärker akzentuiert.
- Benutzer gefielen die grün umrandeten Plus-/Minus-Tasten nicht. Eine neue
  Variante stellt sie deshalb wie in der gelieferten Loxone-Referenz als ruhige
  dunkle Tasten mit großen hellen Zeichen dar; die übrigen grünen Akzente und
  das unveränderte Layout bleiben erhalten.
- Im nächsten Entwurf wurde die Wassertemperatur deutlich kompakter und
  horizontal angeordnet. Der frei gewordene Platz wurde auf ausdrücklichen
  Benutzerwunsch überwiegend dem Betriebsmodus gegeben; dessen Tasten sind nun
  höher und die Beschriftung größer. Der Solltemperaturbereich bleibt kompakt.
- Der Benutzer hat diesen letzten Entwurf als verbindlichen GUI-Zwischenstand
  für die Fortsetzung am nächsten Tag festgelegt. Referenz:
  `pool-control-gui-balanced-draft.html` im threadbezogenen
  Visualisierungsverzeichnis. Noch nicht in die Firmware übernehmen, bis die
  gemeinsame GUI-Abstimmung fortgesetzt wird.
- Anschließend Timeout, MQTT-offline und Reconnect abschließend testen.
- Native Tests erneut ausführen, sobald `gcc/g++` auf dem Host verfügbar ist.
- Veraltete Statusaussagen in `README.md`, `docs/architecture.md` und
  `docs/ui.md` korrigieren: Display, GT911, LVGL und `GuiManager::begin()` sind
  inzwischen im Standardziel integriert und auf Hardware aktiv, werden dort
  aber teilweise noch als deaktiviert oder ausstehend beschrieben.
- Der frühere Entwurf `pool-control-gui-balanced-draft.html` ist im aktuellen
  Thread nicht verfügbar. Auf Basis der erneut gelieferten Gerätefotos und
  drei Loxone-App-Screens wurde deshalb der neue interaktive Entwurf
  `pool-control-loxone-draft.html` im aktuellen Visualisierungsverzeichnis
  erstellt. Der Benutzer hat den final korrigierten Stand als gut bestätigt;
  er ist damit die freigegebene visuelle Referenz für die LVGL-Übernahme.
- Der aktuelle Entwurf wurde auf Benutzerwunsch konsequent dunkler abgestimmt:
  die Wassertemperatur ist flacher, der Betriebsmodus deutlich höher und
  Loxone-inspiriertes Grün erscheint stärker an aktiver Auswahl,
  Verbindungsindikatoren, Statuswerten, Icons und Plus/Minus. Die ruhigen
  dunklen Aktionsflächen und das bestehende Layout bleiben erhalten.
- Nach Benutzerhinweis wurde der Dark Mode anschließend technisch erzwungen,
  sodass Hintergrund, Karten und Buttons nicht mehr das helle Farbschema der
  umgebenden Codex-Oberfläche übernehmen können.
- Der zunächst orange dargestellte Akzent wurde anschließend auf das
  Loxone-inspirierte Grün der Visualisierungspalette umgestellt.
- Für eine bessere Erkennbarkeit der Schaltflächen wurden deren dunkle Flächen
  und Konturen gegenüber reinen Statuskarten kontrastreicher gemacht. Die drei
  Modustasten besitzen zusätzlich eindeutige Symbole; Filterpumpe und
  Sollwerttasten behalten ihre bestehende Power-/Pfeil- beziehungsweise
  Plus-/Minus-Kennzeichnung.
- Plus und Minus wurden auf ausdrücklichen Benutzerwunsch nochmals deutlich
  größer und kräftiger gesetzt; die beiden Sollwerttasten erhielten zusätzlich
  mehr Höhe. Der vergrößerte Betriebsmodusbereich bleibt weitgehend erhalten.
- Ein erster Versuch, den Wert in der sehr flachen Wassertemperatur-Karte nach
  oben zu setzen, schnitt `26,8 °C` unten ab. Nach Benutzer-Screenshot wurde die
  Karte deshalb auf eine stabile horizontale Zeile aus Beschriftung, Wert und
  Thermometer-Icon umgestellt; die kompakte Kartenhöhe bleibt erhalten.

## Wichtige technische Entscheidungen

- Loxone bleibt alleiniger Controller. Das Panel übernimmt Commands erst nach
  einer bestätigenden Statusmeldung.
- Statusmeldungen sind retained, Befehlstopics nicht.
- Bekannte retained Werte bleiben bei bestehender MQTT-Verbindung bedienbar;
  unbekannte Werte und MQTT-offline sperren die betroffenen Controls.
- `Disable Cache` stellt bei Modusbefehlen auch die Weiterleitung identischer
  Werte sicher, ohne einen ungültigen Resetwert `0` zu erzeugen.
- Private Gerätewerte liegen nur in der ignorierten `include/PoolConfig.h`.
- `isHeating` ist ohne eigenständiges Loxone-Signal redundant. Heizpumpe und
  Heizfreigabe bleiben die beiden verbindlichen Heizstatuswerte.

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
- Benutzer hat den digitalen virtuellen Ausgang für den tatsächlichen
  Filterpumpenstatus eingerichtet. Read-only Brokerprüfung erhielt jedoch noch
  keine retained Meldung auf `pool/status/filterPump`; der Ausgang stand beim
  Speichern vermutlich unverändert und löste weder EIN noch AUS aus.
- Benutzer stellte fest, dass die zugehörige Loxone-Filterpumpenlogik noch
  überarbeitet werden muss. Filtertest ist deshalb bewusst pausiert; keine
  reale Pumpe wurde durch Codex geschaltet.
- Loxone-Screenshot zur Filteranforderung analysiert: `pool_cmd_filterPump`
  liegt am `On`-Eingang des Schalter-Bausteins. Dieser Eingang setzt nur EIN;
  ein MQTT-Wert `0` schaltet den Ausgang nicht wieder AUS. Im gezeigten Zustand
  stehen Schalter- und UND-Ausgang bereits auf `Ein`, obwohl der virtuelle
  Eingang aktuell `Aus` anzeigt. Für eine deterministische 0/1-Abbildung sind
  getrennte EIN-/AUS-Flanken oder eine direkte Zustandslogik erforderlich.
- Benutzer hat `pool_cmd_filterPump` in Loxone über eine Flankenerkennung in
  einen Impuls umgewandelt. Noch zu prüfen ist, ob positive und negative Flanke
  getrennt auf `On` beziehungsweise `Off` des Schalter-Bausteins geführt sind.
- Nach Benutzerfreigabe wurde der erste Live-Testbefehl
  `pool/cmd/filterPump = 1` nicht-retained erfolgreich an den LoxBerry-Broker
  publiziert. Der Benutzer bestätigte daraufhin den EIN-Zustand des
  Schalterausgangs; die positive Flanke funktioniert.
- Anschließend wurde `pool/cmd/filterPump = 0` nicht-retained erfolgreich an
  den Broker publiziert. Der Benutzer bestätigte den AUS-Zustand; damit sind
  positive und negative Flanke der Loxone-Schalterlogik erfolgreich geprüft.
- Read-only Brokerprüfung nach dem Ausschalten erfolgreich:
  `pool/status/filterPump = 0` wurde retained empfangen. Der Statusausgang ist
  damit an den Broker angebunden und bildet zumindest den AUS-Zustand korrekt
  ab.
- Benutzer bestätigte, dass die Filterkachel am Panel den retained
  Filterpumpenstatus `0` korrekt als AUS anzeigt. Der komplette AUS-Rückweg von
  Loxone über MQTT bis zum Display ist damit bestätigt.
- Panel-End-to-End-Test für EIN erfolgreich: Benutzer tippte die Filterkachel
  im manuellen Modus; Loxone schaltete ein und die bestätigte Anzeige am Panel
  funktionierte.
- Panel-End-to-End-Test für AUS ebenfalls erfolgreich: Ein zweiter Tipp
  schaltete über MQTT und Loxone wieder aus, die Rückmeldung wurde am Display
  korrekt angezeigt. Die Filterpumpensteuerung ist damit in beiden Richtungen
  vollständig geprüft; Abschlusszustand ist AUS.
- Digitaler Loxone-Statusausgang für `pool/status/heatingPump` wurde vom
  Benutzer eingerichtet. Eine anschließende read-only Brokerabfrage über fünf
  Sekunden erhielt noch keine retained Meldung. Wahrscheinlich hat sich der
  angeschlossene Heizpumpenausgang seit dem Konfigurationsdownload noch nicht
  geändert und deshalb keinen EIN-/AUS-Befehl ausgelöst.
- Nach kontrolliertem Auslösen durch den Benutzer wurde die read-only Abfrage
  wiederholt und erhielt retained `pool/status/heatingPump = 1`. Der
  Heizpumpen-Statusweg von Loxone bis zum Broker funktioniert. Der Benutzer
  bestätigte sowohl die korrekte EIN-Anzeige am Panel als auch, dass der reale
  EIN-Zustand beabsichtigt ist. Ein erzwungener AUS-Test wird deshalb vermieden
  und bei der nächsten regulären Abschaltung nachgeholt.
- Benutzer hat `pool/status/heatingAllowed` mit dem tatsächlichen
  Heizfreigabesignal als digitalen retained Status angebunden und bestätigt,
  dass die Integration funktioniert.
- Nutzen von `pool/status/isHeating` geprüft: Der Wert steuert ausschließlich
  das zusätzliche `HEIZT`-Badge in der GUI und beeinflusst weder Controls noch
  Regelung. Da Loxone kein vom Heizpumpenstatus verschiedenes benötigtes Signal
  liefert, wurde entschieden, den redundanten Status vollständig zu entfernen.
- `isHeating` vollständig aus Topicvertrag, Subscription, Zustandsmodell,
  Status-Updater, GUI/Badge, serieller Diagnose, Simulator, Tests und
  Dokumentation entfernt. Der verbindliche Vertrag umfasst jetzt sechs
  retained Statustopics.
- Referenzsuche nach allen `isHeating`-Varianten ohne Treffer; Simulator-CLI
  mit `--help` erfolgreich gestartet; `git diff --check` ohne Fehler.
- Haupt-Firmwareziel `esp32-s3-panel` erfolgreich gebaut: RAM 31,3 %
  (102532/327680 Bytes), Flash 19,0 % (1247550/6553600 Bytes). Der erste
  Sandbox-Build scheiterte an Windows-Zugriffsfehler 5 im Compiler; derselbe
  Build außerhalb der Sandbox war erfolgreich.
- Aktuelles Haupt-Firmwareziel anschließend erfolgreich auf `COM3` geladen;
  Bootloader, Partitionen und Firmware wurden geschrieben, alle Flash-Hashes
  verifiziert und das Board per RTS neu gestartet.
- Benutzer bestätigte nach dem Neustart die normale Oberfläche und dass das
  entfernte zusätzliche Heiz-Badge nicht mehr angezeigt wird. Die
  `isHeating`-Bereinigung ist damit auch auf der Hardware verifiziert.
- Benutzer hat `pool/status/waterTemp` mit dem tatsächlichen Messwert als
  analogen retained Status angebunden und die korrekte Anzeige bestätigt.
  Read-only Brokerprüfung erhielt retained `pool/status/waterTemp = 26.8`;
  Topic, Payload und Rückweg zum Panel sind damit verifiziert.
- Hardwarefoto der aktuellen GUI bewertet: Gesamtaufteilung funktioniert,
  aber Plus/Minus haben unzureichende Zeichensichtbarkeit und die Schriften
  sind teilweise zu klein. Noch keine Codeänderung vorgenommen.
- Zwei Loxone-App-Screens als Referenz bewertet; ebenfalls keine GUI-
  Codeänderung vorgenommen.
- Weiteres Hardwarefoto bewertet und Layout-Erhalt sowie bessere visuelle
  Bedienkennzeichnung der Filterpumpen-Kachel dokumentiert; kein Code geändert.
- Interaktiver GUI-Entwurf unter dem threadbezogenen Visualisierungsverzeichnis
  erstellt und erfolgreich als vollständige Vorschau gerendert; weiterhin
  keine Firmware- oder Projekt-GUI-Datei geändert.
- Dunkle Entwurfsvariante ebenfalls erfolgreich als vollständige Vorschau
  gerendert; weiterhin keine Firmwareänderung.
- Dunkle Variante mit verstärkten grünen Akzenten ebenfalls erfolgreich als
  vollständige Vorschau gerendert; weiterhin keine Firmwareänderung.
- Überarbeitete Variante mit neutralen Plus-/Minus-Tasten ebenfalls erfolgreich
  gerendert; weiterhin keine Firmwareänderung.
- Variante mit kompakter Wassertemperatur und deutlich mehr Platz für den
  Betriebsmodus erfolgreich gerendert; weiterhin keine Firmwareänderung.
- Native Tests nicht ausgeführt; dem Host fehlt `gcc/g++`.
- Aktualisierungssichtung am 20.07.2026: Git-Historie seit `0bfa87a`,
  Build-Konfiguration, AppController, Display-/Touch-/LVGL-Integration,
  GUI- und Bedienpolicy, MQTT-Vertrag, Projektdokumentation und registrierte
  Tests geprüft. `git diff --check` war fehlerfrei. Keine Builds, Native Tests
  oder Hardwaretests ausgeführt; `gcc/g++` sind weiterhin nicht verfügbar.
- Drei Loxone-App-Referenzen und zwei aktuelle Gerätefotos visuell geprüft.
  Der neue interaktive 480×480-Entwurf übernimmt die Loxone-Prinzipien aus
  dunklen ruhigen Karten, klarer Wertehierarchie, separaten Aktionsflächen und
  sparsamen Akzenten. Modus, Sollwert und Filterpumpenstatus reagieren lokal;
  Firmware, MQTT und reale Hardware wurden dabei nicht verändert oder getestet.

## Nächster konkreter Schritt

Den freigegebenen Stand aus `pool-control-loxone-draft.html` in
`lib/Gui/GuiManager.cpp` übertragen, ohne MQTT-, Command- oder Bedienpolicy zu
ändern; anschließend bauen, auf `COM3` flashen und visuell sowie per Touch
prüfen. Danach Stabilität und korrekte Quelle von `pool/status/targetTemp`,
Timeout, MQTT-offline und Reconnect prüfen; Heizpumpenstatus `0` bei der
nächsten regulären Abschaltung kontrollieren.
