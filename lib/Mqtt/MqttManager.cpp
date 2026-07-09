#include "MqttManager.h"

#include <WiFi.h>
#include "WifiManager.h"
#include "Topics.h"
#include "PoolConfig.h"

namespace
{
    bool parseFloatPayload(const String& value, float& result)
    {
        char* end = nullptr;
        result = strtof(value.c_str(), &end);
        return end != value.c_str() && *end == '\0' && isfinite(result);
    }

    bool parseBoolPayload(const String& value, bool& result)
    {
        if (value == "1" || value == "true")
        {
            result = true;
            return true;
        }
        if (value == "0" || value == "false")
        {
            result = false;
            return true;
        }
        return false;
    }
}

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
        if (!parseFloatPayload(value, _state.waterTemperature))
        {
            Serial.printf("[MQTT] invalid water temperature ignored: %s\n", value.c_str());
            return;
        }
        _state.hasWaterTemperature = true;
    }
    else if (strcmp(topic, Topics::Status::TargetTemperature) == 0)
    {
        if (!parseFloatPayload(value, _state.targetTemperature))
        {
            Serial.printf("[MQTT] invalid target temperature ignored: %s\n", value.c_str());
            return;
        }
        _state.hasTargetTemperature = true;
    }
    else if (strcmp(topic, Topics::Status::FilterPump) == 0)
    {
        if (!parseBoolPayload(value, _state.filterPump))
        {
            Serial.printf("[MQTT] invalid filter pump state ignored: %s\n", value.c_str());
            return;
        }
        _state.hasFilterPump = true;
    }
    else if (strcmp(topic, Topics::Status::HeatingPump) == 0)
    {
        if (!parseBoolPayload(value, _state.heatingPump))
        {
            Serial.printf("[MQTT] invalid heating pump state ignored: %s\n", value.c_str());
            return;
        }
        _state.hasHeatingPump = true;
    }
    else if (strcmp(topic, Topics::Status::HeatingAllowed) == 0)
    {
        if (!parseBoolPayload(value, _state.heatingAllowed))
        {
            Serial.printf("[MQTT] invalid heating permission ignored: %s\n", value.c_str());
            return;
        }
        _state.hasHeatingAllowed = true;
    }
    else if (strcmp(topic, Topics::Status::Mode) == 0)
    {
        const int mode = value.toInt();
        if (mode < static_cast<int>(PoolMode::Off) ||
            mode > static_cast<int>(PoolMode::Heating))
        {
            Serial.printf("[MQTT] invalid pool mode ignored: %s\n", value.c_str());
            return;
        }

        _state.mode = static_cast<PoolMode>(mode);
        _state.hasMode = true;
    }
    else
    {
        return;
    }

    _state.lastStatusUpdateAt = millis();
}

void MqttManager::mqttCallback(char* topic, byte* payload, unsigned int length)
{
    if (_instance)
    {
        _instance->handleMessage(topic, payload, length);
    }
}
