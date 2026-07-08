#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include "PoolState.h"

class WifiManager;

class MqttManager
{
public:
    explicit MqttManager(PoolState& state);

    void begin(WifiManager& wifi);
    void loop();

    bool isConnected() const;

    void publishHeartbeat();
    void publishAvailability(bool online);

    void sendMode(uint8_t mode);
    void sendTargetTemperature(float value);
    void sendFilterPump(bool on);

private:
    PoolState& _state;
    WifiManager* _wifi = nullptr;

    WiFiClient _wifiClient;
    PubSubClient _client;

    uint32_t _lastReconnectAttempt = 0;
    uint32_t _lastHeartbeat = 0;

    void connect();
    void subscribeTopics();
    void handleMessage(char* topic, uint8_t* payload, unsigned int length);

    static MqttManager* _instance;
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
};