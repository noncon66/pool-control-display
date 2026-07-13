#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "PanelControlPolicy.h"
#include "PanelCommandState.h"
#include "PoolState.h"

class WifiManager;

class MqttManager
{
public:
    // Speichert eine Referenz auf den PoolState des AppControllers.
    explicit MqttManager(PoolState& state);

    // begin() konfiguriert MQTT einmalig; loop() hält die Verbindung aktiv und
    // verarbeitet eingehende Nachrichten.
    void begin(WifiManager& wifi);
    void loop();

    bool isConnected() const;

    // Technische Statusmeldungen des Panels selbst.
    void publishHeartbeat();
    void publishAvailability(bool online);

    // Diese Methoden veröffentlichen nur Wünsche. Sie ändern PoolState bewusst
    // nicht; ausschließlich Loxone-Statusmeldungen bestätigen neue Zustände.
    bool sendMode(uint8_t mode);
    bool sendTargetTemperature(float value);
    bool sendFilterPump(bool on);

    bool canSendModeCommand() const;

    // Kann von der späteren GUI verwendet werden, um den Filterpumpen-Schalter
    // nur bei bestätigtem, aktuellem Manual-Modus freizugeben.
    bool canSendFilterPumpCommand() const;
    bool canSendTargetTemperatureCommand(float value) const;

    // Die spätere GUI kann hier ablesen, ob ein Bedienwunsch noch wartet,
    // bestätigt wurde oder in ein Timeout gelaufen ist.
    const PanelCommandState& commandState() const { return _commands; }

private:
    // Referenz auf den gemeinsamen Zustand, keine separate Kopie.
    PoolState& _state;

    // Wird erst an begin() übergeben und deshalb als Zeiger gespeichert.
    WifiManager* _wifi = nullptr;

    // PubSubClient verwendet WiFiClient als TCP-Transport.
    WiFiClient _wifiClient;
    PubSubClient _client;
    PanelCommandState _commands;

    // Zeitstempel verhindern Neuverbindungen und Heartbeats in jedem loop().
    uint32_t _lastReconnectAttempt = 0;
    uint32_t _lastHeartbeat = 0;

    void connect();
    void subscribeTopics();
    void handleMessage(char* topic, uint8_t* payload, unsigned int length);
    bool publishCommand(const char* topic, const char* payload);

    // PubSubClient verlangt einen statischen Callback. Über _instance kann
    // dieser die Nachricht an das aktive Objekt weiterreichen.
    static MqttManager* _instance;
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
};
