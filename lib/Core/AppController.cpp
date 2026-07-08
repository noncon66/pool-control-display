#include "AppController.h"

AppController::AppController()
    : _mqtt(_state)
{
}

void AppController::begin()
{
    Serial.println("=== Pool Control Display ===");
    Serial.println("Starting application...");

    _wifi.begin();
    _mqtt.begin(_wifi);

    _dashboard.begin();
    _dashboard.renderFull(_state, _wifi, _mqtt);

    _lastState = _state;
}

void AppController::loop()
{
    _wifi.loop();
    _mqtt.loop();

    updateConnectivityState();
    handleStateChanges();

    const uint32_t now = millis();
    if (now - _lastPeriodicDashboardUpdate >= 30000)
    {
        _lastPeriodicDashboardUpdate = now;
        _dashboard.renderFull(_state, _wifi, _mqtt);
    }

    delay(10);
}

void AppController::updateConnectivityState()
{
    _state.wifiConnected = _wifi.isConnected();
    _state.mqttConnected = _mqtt.isConnected();
}

void AppController::handleStateChanges()
{
    if (!hasStateChanged())
    {
        return;
    }

    _dashboard.renderDiff(_lastState, _state, _wifi, _mqtt);
    _lastState = _state;
}

bool AppController::hasStateChanged() const
{
    return
        _state.waterTemperature != _lastState.waterTemperature ||
        _state.targetTemperature != _lastState.targetTemperature ||
        _state.filterPump != _lastState.filterPump ||
        _state.heatingPump != _lastState.heatingPump ||
        _state.heatingAllowed != _lastState.heatingAllowed ||
        _state.mode != _lastState.mode ||
        _state.wifiConnected != _lastState.wifiConnected ||
        _state.mqttConnected != _lastState.mqttConnected;
}