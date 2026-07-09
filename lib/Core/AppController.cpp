#include "AppController.h"
#include "PanelViewModel.h"

AppController::AppController()
    : _mqtt(_state)
{
    // MqttManager erhält eine Referenz auf _state. Empfangene MQTT-Werte
    // können so ohne zusätzliche Kopie in den gemeinsamen Zustand geschrieben
    // werden.
}

void AppController::begin()
{
    Serial.println("=== Pool Control Display ===");
    Serial.println("Starting application...");

    // Zuerst WLAN initialisieren, danach den WLAN-Manager an MQTT übergeben.
    _mqtt.begin(_wifi);

    _dashboard.begin();
    _dashboard.renderFull(_state, _wifi, _mqtt);

    _lastState = _state;
}

void AppController::loop()
{
    // Beide loop()-Methoden vermeiden lange Wartezeiten. Sie halten bestehende
    // Verbindungen aktiv oder starten bei Bedarf einen neuen Verbindungsversuch.
    _wifi.loop();
    _mqtt.loop();

    // Verbindungszustände übernehmen und nur bei tatsächlichen Änderungen
    // eine Differenzausgabe erzeugen.
    updateConnectivityState();
    handleStateChanges();

    const uint32_t now = millis();
    // Die Subtraktion bleibt auch beim Überlauf von millis() korrekt, solange
    // uint32_t verwendet wird.
    if (now - _lastPeriodicDashboardUpdate >= 30000)
    {
        _lastPeriodicDashboardUpdate = now;
        _dashboard.renderFull(_state, _wifi, _mqtt);
    }

    // Eine kurze Pause senkt die CPU-Last, ohne MQTT merklich auszubremsen.
    delay(10);
}

void AppController::updateConnectivityState()
{
    _state.wifiConnected = _wifi.isConnected();
    _state.mqttConnected = _mqtt.isConnected();
}

void AppController::handleStateChanges()
{
    // Ohne Zustandsänderung ist keine neue Ausgabe erforderlich.
    if (!hasStateChanged())
    {
        return;
    }

    _dashboard.renderDiff(_lastState, _state, _wifi, _mqtt);
    // Nach der Ausgabe wird der aktuelle Zustand zur neuen Vergleichsbasis.
    _lastState = _state;
}

bool AppController::hasStateChanged() const
{
    // Der gesamte Ausdruck wird true, sobald ein Feld abweicht. Auch die
    // has...-Felder zählen: Ein erstmals empfangener Wert 0 ist ebenfalls eine
    // wichtige Zustandsänderung.
    return
        _state.waterTemperature != _lastState.waterTemperature ||
        _state.targetTemperature != _lastState.targetTemperature ||
        _state.filterPump != _lastState.filterPump ||
        _state.heatingPump != _lastState.heatingPump ||
        _state.heatingAllowed != _lastState.heatingAllowed ||
        _state.isHeating != _lastState.isHeating ||
        _state.mode != _lastState.mode ||
        _state.hasWaterTemperature != _lastState.hasWaterTemperature ||
        _state.hasTargetTemperature != _lastState.hasTargetTemperature ||
        _state.hasFilterPump != _lastState.hasFilterPump ||
        _state.hasHeatingPump != _lastState.hasHeatingPump ||
        _state.hasHeatingAllowed != _lastState.hasHeatingAllowed ||
        _state.hasIsHeating != _lastState.hasIsHeating ||
        _state.hasMode != _lastState.hasMode ||
        _state.wifiConnected != _lastState.wifiConnected ||
        _state.mqttConnected != _lastState.mqttConnected;
}
