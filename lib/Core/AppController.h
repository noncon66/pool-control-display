#pragma once

#include <Arduino.h>
#include "PoolState.h"
#include "ScreenPowerPolicy.h"
#include "WifiManager.h"
#include "MqttManager.h"
#include "OtaManager.h"
#include "GuiManager.h"
#include "DisplayManager.h"
#include "SerialDashboard.h"

class AppController
{
public:
    AppController();

    // begin() initialisiert die Anwendung einmalig. loop() wird fortlaufend
    // aus der Arduino-loop() aufgerufen.
    void begin();
    void loop();

private:
    // _state enthält den aktuellen, von Loxone bestätigten Zustand.
    // _lastState ist die vorherige Kopie zum Erkennen von Änderungen.
    PoolState _state;
    PoolState _lastState;

    // Jeder Manager kapselt einen klar abgegrenzten Aufgabenbereich.
    WifiManager _wifi;
    MqttManager _mqtt;
    OtaManager _ota;
    DisplayManager _display;
    // PWM dimming visibly flickers on this panel across the tested duty/frequency
    // range. Keep full brightness until the stable black/off transition.
    ScreenPowerPolicy _screenPower{false};
    GuiManager _gui;
    SerialDashboard _dashboard;

    // Zeitpunkt der letzten vollständigen Dashboard-Ausgabe.
    uint32_t _lastPeriodicDashboardUpdate = 0;

    // Diese Hilfsfunktionen werden nur innerhalb dieser Klasse verwendet.
    void updateConnectivityState();
    void updateScreenPower(uint32_t now);
    void handleStateChanges();
    bool hasStateChanged() const;
};
