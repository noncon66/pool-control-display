#include "MqttManager.h"

#include <WiFi.h>
#include "WifiManager.h"
#include "Topics.h"
#include "PoolConfig.h"

MqttManager* MqttManager::_instance = nullptr;

MqttManager::MqttManager(PoolState& state)
    : _state(state), _client(_wifiClient)
{
}

void MqttManager::begin(WifiManager& wifi)
{
    _wifi = &wifi;
    _instance = this;

    _client.setServer(MQTT_HOST, MQTT_PORT);
    _client.setCallback(MqttManager::mqttCallback);
}

void MqttManager::loop()
{
    if (!_wifi || !_wifi->isConnected())
    {
        return;
    }

    if (!_client.connected())
    {
        const uint32_t now = millis();
        if (now - _lastReconnectAttempt >= 5000)
        {
            _lastReconnectAttempt = now;
            connect();
        }
        return;
    }

    _client.loop();

    const uint32_t now = millis();
    if (now - _lastHeartbeat >= 30000)
    {
        _lastHeartbeat = now;
        publishHeartbeat();
    }
}

bool MqttManager::isConnected() const
{
    // _client.connected() is not a const method on the client, so cast away const-ness here
    return const_cast<MqttManager*>(this)->_client.connected();
}

void MqttManager::publishHeartbeat()
{
    _client.publish(Topics::Device::Heartbeat, "alive", true);
}

void MqttManager::publishAvailability(bool online)
{
    _client.publish(Topics::Device::Availability, online ? "online" : "offline", true);
}

bool MqttManager::sendMode(uint8_t mode)
{
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%u", mode);
    return publishCommand(Topics::Command::SetMode, buffer);
}

bool MqttManager::sendTargetTemperature(float value)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.1f", value);
    return publishCommand(Topics::Command::SetTargetTemp, buffer);
}

bool MqttManager::sendFilterPump(bool on)
{
    return publishCommand(Topics::Command::SetFilterPump, on ? "1" : "0");
}

bool MqttManager::publishCommand(const char* topic, const char* payload)
{
    if (!_client.connected())
    {
        Serial.printf("[MQTT] command dropped while disconnected: %s\n", topic);
        return false;
    }

    if (!_client.publish(topic, payload, false))
    {
        Serial.printf("[MQTT] command publish failed: %s\n", topic);
        return false;
    }

    return true;
}

void MqttManager::connect()
{
    Serial.printf("[MQTT] connecting to %s:%d\n", MQTT_HOST, MQTT_PORT);

    bool ok = false;
    if (strlen(MQTT_USERNAME) > 0)
    {
        ok = _client.connect(
            DEVICE_NAME,
            MQTT_USERNAME,
            MQTT_PASSWORD,
            Topics::Device::Availability,
            1,
            true,
            "offline"
        );
    }
    else
    {
        ok = _client.connect(
            DEVICE_NAME,
            Topics::Device::Availability,
            1,
            true,
            "offline"
        );
    }

    if (!ok)
    {
        Serial.printf("[MQTT] connect failed, rc=%d\n", _client.state());
        return;
    }

    Serial.println("[MQTT] connected");
    publishAvailability(true);
    subscribeTopics();
}

void MqttManager::subscribeTopics()
{
    _client.subscribe(Topics::Status::WaterTemperature);
    _client.subscribe(Topics::Status::TargetTemperature);
    _client.subscribe(Topics::Status::FilterPump);
    _client.subscribe(Topics::Status::HeatingPump);
    _client.subscribe(Topics::Status::HeatingAllowed);
    _client.subscribe(Topics::Status::Mode);

    Serial.println("[MQTT] subscriptions complete");
}

void MqttManager::handleMessage(char* topic, uint8_t* payload, unsigned int length)
{
    String value;
    value.reserve(length);
    for (unsigned int i = 0; i < length; ++i)
    {
        value += static_cast<char>(payload[i]);
    }

    Serial.printf("[MQTT] %s = %s\n", topic, value.c_str());

    if (strcmp(topic, Topics::Status::WaterTemperature) == 0)
    {
        _state.waterTemperature = value.toFloat();
    }
    else if (strcmp(topic, Topics::Status::TargetTemperature) == 0)
    {
        _state.targetTemperature = value.toFloat();
    }
    else if (strcmp(topic, Topics::Status::FilterPump) == 0)
    {
        _state.filterPump = (value == "1" || value == "true");
    }
    else if (strcmp(topic, Topics::Status::HeatingPump) == 0)
    {
        _state.heatingPump = (value == "1" || value == "true");
    }
    else if (strcmp(topic, Topics::Status::HeatingAllowed) == 0)
    {
        _state.heatingAllowed = (value == "1" || value == "true");
    }
    else if (strcmp(topic, Topics::Status::Mode) == 0)
    {
        _state.mode = static_cast<PoolMode>(value.toInt());
    }
}

void MqttManager::mqttCallback(char* topic, byte* payload, unsigned int length)
{
    if (_instance)
    {
        _instance->handleMessage(topic, payload, length);
    }
}
