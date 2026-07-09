#include "MqttManager.h"

#include <WiFi.h>
#include "WifiManager.h"
#include "Topics.h"
#include "PoolConfig.h"

namespace
{
    // String::toFloat() liefert sowohl für "0" als auch für ungültigen Text 0.
    // strtof() meldet zusätzlich, an welcher Stelle die Auswertung endete.
    bool parseFloatPayload(const String& value, float& result)
    {
        char* end = nullptr;
        result = strtof(value.c_str(), &end);
        // Ein gültiger Wert muss eine Zahl enthalten, die gesamte Nachricht
        // verbrauchen und endlich sein (weder NaN noch unendlich).
        return end != value.c_str() && *end == '\0' && isfinite(result);
    }

    // Nur ausdrücklich unterstützte boolesche Darstellungen akzeptieren.
    // Ungültiger Text darf nicht unbemerkt zu einem AUS-Status werden.
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
    // Die Initialisierungsliste speichert die Zustandsreferenz und verbindet
    // PubSubClient mit seinem WLAN-Transport.
}

void MqttManager::begin(WifiManager& wifi)
{
    _wifi = &wifi;
    _instance = this;

    // Brokeradresse speichern und den statischen Nachrichten-Callback anmelden.
    _client.setServer(MQTT_HOST, MQTT_PORT);
    _client.setCallback(MqttManager::mqttCallback);
}

void MqttManager::loop()
{
    // Timeouts müssen auch dann weiterlaufen, wenn WLAN oder MQTT ausfällt.
    _commands.updateTimeouts(millis());

    // MQTT funktioniert nicht ohne WLAN. WifiManager kümmert sich selbst um
    // Neuverbindungen; bis dahin ist hier nichts zu tun.
    if (!_wifi || !_wifi->isConnected())
    {
        return;
    }

    if (!_client.connected())
    {
        const uint32_t now = millis();
        // Broker-Verbindungsversuche auf einen alle fünf Sekunden begrenzen.
        if (now - _lastReconnectAttempt >= 5000)
        {
            _lastReconnectAttempt = now;
            connect();
        }
        return;
    }

    // Nachrichten empfangen, Callbacks ausführen und die Verbindung aktiv
    // halten. PubSubClient benötigt diesen Aufruf regelmäßig.
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
    // PubSubClient::connected() ist nicht als const deklariert, obwohl es nur
    // einen Zustand abfragt. Der Cast ermöglicht hier trotzdem eine const-API.
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
    // Das ist Protokollprüfung und keine Poollogik: Gültige Betriebsmodi sind
    // ausschließlich Off, Auto und Manual. Der Heizstatus ist ein separates
    // boolesches Statusfeld und niemals ein Modusbefehl.
    if (mode > static_cast<uint8_t>(PoolMode::Manual))
    {
        Serial.printf("[MQTT] invalid mode command rejected: %u\n", mode);
        return false;
    }

    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%u", mode);
    if (!publishCommand(Topics::Command::SetMode, buffer))
    {
        return false;
    }

    _commands.markModePending(static_cast<PoolMode>(mode), millis());
    return true;
}

bool MqttManager::sendTargetTemperature(float value)
{
    // Das Panel bietet die Änderung nur im bestätigten Automatikmodus und nur
    // innerhalb der vereinbarten Werte an. Loxone prüft den Wunsch zusätzlich.
    if (!canSendTargetTemperatureCommand(value))
    {
        Serial.println("[MQTT] target temperature command rejected: invalid value, mode or connection state");
        return false;
    }

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.1f", value);
    if (!publishCommand(Topics::Command::SetTargetTemp, buffer))
    {
        return false;
    }

    _commands.markTargetTemperaturePending(value, millis());
    return true;
}

bool MqttManager::sendFilterPump(bool on)
{
    // Das Panel verhindert eine offensichtlich unzulässige Bedienung bereits
    // in der Oberfläche bzw. beim Senden. Das ersetzt ausdrücklich nicht die
    // verbindliche Prüfung innerhalb der Loxone-Steuerung.
    if (!canSendFilterPumpCommand())
    {
        Serial.println("[MQTT] filter pump command rejected: Manual mode not confirmed or data stale");
        return false;
    }

    if (!publishCommand(Topics::Command::SetFilterPump, on ? "1" : "0"))
    {
        return false;
    }

    _commands.markFilterPumpPending(on, millis());
    return true;
}

bool MqttManager::canSendFilterPumpCommand() const
{
    return PanelControlPolicy::canControlFilterPump(
        _state,
        isConnected(),
        millis());
}

bool MqttManager::canSendTargetTemperatureCommand(float value) const
{
    return PanelControlPolicy::canAdjustTargetTemperature(
               _state,
               isConnected(),
               millis()) &&
           PanelControlPolicy::isValidTargetTemperature(value);
}

bool MqttManager::publishCommand(const char* topic, const char* payload)
{
    // Befehle werden lokal nicht zwischengespeichert. Ein Offline-Wunsch wird
    // verworfen, damit er nicht unerwartet nach einer Neuverbindung ausgeführt
    // wird.
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
    // Broker-Zugangsdaten nur verwenden, wenn ein Benutzername eingetragen ist.
    if (strlen(MQTT_USERNAME) > 0)
    {
        // Die letzten Argumente konfigurieren das MQTT Last Will and Testament.
        // Verschwindet das Panel unerwartet, veröffentlicht der Broker
        // stellvertretend den gespeicherten Status "offline".
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
    // Eine eventuell gespeicherte "offline"-Meldung ersetzen und anschließend
    // alle von Loxone veröffentlichten Werte abonnieren.
    publishAvailability(true);
    subscribeTopics();
}

void MqttManager::subscribeTopics()
{
    // Abonnements nach jeder Neuverbindung wiederholen, weil der Client eine
    // neue, saubere MQTT-Sitzung startet.
    _client.subscribe(Topics::Status::WaterTemperature);
    _client.subscribe(Topics::Status::TargetTemperature);
    _client.subscribe(Topics::Status::FilterPump);
    _client.subscribe(Topics::Status::HeatingPump);
    _client.subscribe(Topics::Status::HeatingAllowed);
    _client.subscribe(Topics::Status::IsHeating);
    _client.subscribe(Topics::Status::Mode);

    Serial.println("[MQTT] subscriptions complete");
}

void MqttManager::handleMessage(char* topic, uint8_t* payload, unsigned int length)
{
    // MQTT-Nutzdaten sind Byte-Arrays und enden nicht garantiert mit '\0'.
    // Vor der Auswertung deshalb exakt 'length' Bytes in einen String kopieren.
    String value;
    value.reserve(length);
    for (unsigned int i = 0; i < length; ++i)
    {
        value += static_cast<char>(payload[i]);
    }

    Serial.printf("[MQTT] %s = %s\n", topic, value.c_str());

    // Thema zuordnen, Nutzdaten prüfen und nur das zugehörige Zustandsfeld
    // aktualisieren. Diese Funktion trifft keine Pool-Steuerungsentscheidung.
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
        _commands.confirmTargetTemperature(_state.targetTemperature, millis());
    }
    else if (strcmp(topic, Topics::Status::FilterPump) == 0)
    {
        if (!parseBoolPayload(value, _state.filterPump))
        {
            Serial.printf("[MQTT] invalid filter pump state ignored: %s\n", value.c_str());
            return;
        }
        _state.hasFilterPump = true;
        _commands.confirmFilterPump(_state.filterPump, millis());
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
    else if (strcmp(topic, Topics::Status::IsHeating) == 0)
    {
        if (!parseBoolPayload(value, _state.isHeating))
        {
            Serial.printf("[MQTT] invalid heating status ignored: %s\n", value.c_str());
            return;
        }
        _state.hasIsHeating = true;
    }
    else if (strcmp(topic, Topics::Status::Mode) == 0)
    {
        const int mode = value.toInt();
        if (mode < static_cast<int>(PoolMode::Off) ||
            mode > static_cast<int>(PoolMode::Manual))
        {
            Serial.printf("[MQTT] invalid pool mode ignored: %s\n", value.c_str());
            return;
        }

        _state.mode = static_cast<PoolMode>(mode);
        _state.hasMode = true;
        _commands.confirmMode(_state.mode, millis());
    }
    else
    {
        return;
    }

    // Nur gültige und bekannte Statusmeldungen aktualisieren diesen Zeitstempel.
    _state.lastStatusUpdateAt = millis();
}

void MqttManager::mqttCallback(char* topic, byte* payload, unsigned int length)
{
    // Den statischen PubSubClient-Callback an die aktive Managerinstanz leiten.
    if (_instance)
    {
        _instance->handleMessage(topic, payload, length);
    }
}
