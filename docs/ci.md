# Automatische Builds und Tests

Der Workflow `.github/workflows/ci.yml` läuft bei jedem Push und bei jedem
Pull Request auf einem GitHub-Linux-Runner.

Er führt folgende Prüfungen aus:

1. PlatformIO 6.1.19 installieren
2. Private Konfiguration durch die eingecheckte Beispielkonfiguration ersetzen
3. ESP32-S3-Firmware vollständig bauen
4. Hardwareunabhängige Unity-Tests im nativen PlatformIO-Ziel ausführen
5. Syntax des Python-MQTT-Simulators prüfen

## Warum die Beispielkonfiguration verwendet wird

`include/PoolConfig.h` enthält später private WLAN- und MQTT-Zugangsdaten und
wird deshalb nicht eingecheckt. Für den Build benötigt der Compiler die Datei
trotzdem. Der Workflow kopiert daher `PoolConfig.example.h` nur innerhalb des
temporären GitHub-Runners.

## Ergebnis prüfen

Der Status ist im GitHub-Repository unter **Actions** sichtbar. Ein rotes
Ergebnis bedeutet, dass mindestens Build, Test oder Syntaxprüfung fehlgeschlagen
ist. Der Commit sollte dann nicht als geprüfter Stand verwendet werden.

## Lokale Entsprechung

ESP32-Build:

```sh
pio run -e esp32-s3-panel
```

Hardwareunabhängige Tests:

```sh
pio test -e native
```

Für die nativen Tests muss lokal ein C++-Compiler vorhanden sein. Unter macOS
stellt ihn üblicherweise Xcode Command Line Tools bereit. GitHub Actions besitzt
bereits einen geeigneten Compiler.
