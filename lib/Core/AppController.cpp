#include "AppController.h"
#include "PanelViewModel.h"

namespace
{
    const char* screenPowerName(ScreenPowerLevel level)
    {
        switch (level)
        {
            case ScreenPowerLevel::Awake:
                return "awake";
            case ScreenPowerLevel::Dimmed:
                return "dimmed";
            case ScreenPowerLevel::Off:
                return "off";
            default:
                return "unknown";
        }
    }
}

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

    // Display output and pointer input are active through LVGL. Control
    // interaction remains guarded by the central MQTT/state policy.
    if (!_display.begin())
    {
        Serial.println("[App] WARNING: display hardware initialization failed");
    }
    _screenPower.begin(millis());

    // Zuerst WLAN initialisieren, danach den WLAN-Manager an MQTT übergeben.
    _wifi.begin();
    _mqtt.begin(_wifi);
    _ota.begin(_wifi);

    if (_display.isLvglReady())
    {
        _gui.begin(_state, _mqtt, true);
        Serial.printf(
            "[App] LVGL main screen initialized; touch=%s, MQTT controls=enabled\n",
            _display.isLvglTouchReady() ? "ready" : "disabled");
    }

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
    _ota.loop();
    _display.loop();
    updateScreenPower(millis());

    // Verbindungszustände übernehmen und nur bei tatsächlichen Änderungen
    // eine Differenzausgabe erzeugen.
    updateConnectivityState();
    handleStateChanges();

    const uint32_t now = millis();
    _gui.update(now);
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

void AppController::updateScreenPower(uint32_t now)
{
    const ScreenPowerLevel previousLevel = _screenPower.level();

    if (_display.consumeTouchPress())
    {
        const bool canForwardTouch = _screenPower.handleTouch(now);
        if (!canForwardTouch)
        {
            _display.suppressCurrentTouchForLvgl();
        }
        Serial.println(canForwardTouch
            ? "[Display] active touch; forwarded to LVGL"
            : "[Display] wake touch; control forwarding suppressed");
    }

    const ScreenPowerLevel newLevel = _screenPower.update(now);
    if (newLevel == previousLevel)
    {
        return;
    }

    const uint8_t brightness = _screenPower.brightnessPercent();
    _display.setBacklightPercent(brightness);
    Serial.printf(
        "[Display] power=%s brightness=%u%%\n",
        screenPowerName(newLevel),
        brightness);
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
        _state.mode != _lastState.mode ||
        _state.hasWaterTemperature != _lastState.hasWaterTemperature ||
        _state.hasTargetTemperature != _lastState.hasTargetTemperature ||
        _state.hasFilterPump != _lastState.hasFilterPump ||
        _state.hasHeatingPump != _lastState.hasHeatingPump ||
        _state.hasHeatingAllowed != _lastState.hasHeatingAllowed ||
        _state.hasMode != _lastState.hasMode ||
        _state.wifiConnected != _lastState.wifiConnected ||
        _state.mqttConnected != _lastState.mqttConnected;
}
