#pragma once

#include <stdint.h>
#include <string.h>

#include "MqttPayloadParser.h"
#include "PanelCommandState.h"
#include "PoolState.h"
#include "Topics.h"

enum class StatusUpdateResult : uint8_t
{
    Applied,
    UnknownTopic,
    InvalidPayload
};

// Ordnet eine bestaetigte Loxone-Statusmeldung dem PoolState zu. Die Klasse
// kennt weder Arduino noch den MQTT-Transport und kann deshalb nativ getestet
// werden. Ungueltige oder unbekannte Meldungen veraendern den Zustand nicht.
class PoolStatusUpdater
{
public:
    static StatusUpdateResult apply(
        PoolState& state,
        PanelCommandState& commands,
        const char* topic,
        const char* payload,
        uint32_t now)
    {
        if (!topic)
        {
            return StatusUpdateResult::UnknownTopic;
        }

        if (strcmp(topic, Topics::Status::WaterTemperature) == 0)
        {
            float value = 0.0f;
            if (!MqttPayloadParser::parseFloat(payload, value))
            {
                return StatusUpdateResult::InvalidPayload;
            }
            state.waterTemperature = value;
            state.hasWaterTemperature = true;
        }
        else if (strcmp(topic, Topics::Status::TargetTemperature) == 0)
        {
            float value = 0.0f;
            if (!MqttPayloadParser::parseFloat(payload, value))
            {
                return StatusUpdateResult::InvalidPayload;
            }
            state.targetTemperature = value;
            state.hasTargetTemperature = true;
            state.lastTargetTemperatureUpdateAt = now;
            commands.confirmTargetTemperature(value, now);
        }
        else if (strcmp(topic, Topics::Status::FilterPump) == 0)
        {
            bool value = false;
            if (!parseBool(payload, value))
            {
                return StatusUpdateResult::InvalidPayload;
            }
            state.filterPump = value;
            state.hasFilterPump = true;
            state.lastFilterPumpUpdateAt = now;
            commands.confirmFilterPump(value, now);
        }
        else if (strcmp(topic, Topics::Status::HeatingPump) == 0)
        {
            bool value = false;
            if (!parseBool(payload, value))
            {
                return StatusUpdateResult::InvalidPayload;
            }
            state.heatingPump = value;
            state.hasHeatingPump = true;
        }
        else if (strcmp(topic, Topics::Status::HeatingAllowed) == 0)
        {
            bool value = false;
            if (!parseBool(payload, value))
            {
                return StatusUpdateResult::InvalidPayload;
            }
            state.heatingAllowed = value;
            state.hasHeatingAllowed = true;
        }
        else if (strcmp(topic, Topics::Status::IsHeating) == 0)
        {
            bool value = false;
            if (!parseBool(payload, value))
            {
                return StatusUpdateResult::InvalidPayload;
            }
            state.isHeating = value;
            state.hasIsHeating = true;
        }
        else if (strcmp(topic, Topics::Status::Mode) == 0)
        {
            PoolMode value = PoolMode::Off;
            if (!MqttPayloadParser::parseMode(payload, value))
            {
                return StatusUpdateResult::InvalidPayload;
            }
            state.mode = value;
            state.hasMode = true;
            state.lastModeUpdateAt = now;
            commands.confirmMode(value, now);
        }
        else
        {
            return StatusUpdateResult::UnknownTopic;
        }

        state.lastStatusUpdateAt = now;
        return StatusUpdateResult::Applied;
    }

private:
    static bool parseBool(const char* payload, bool& result)
    {
        if (!payload)
        {
            return false;
        }
        if (strcmp(payload, "1") == 0 || strcmp(payload, "true") == 0)
        {
            result = true;
            return true;
        }
        if (strcmp(payload, "0") == 0 || strcmp(payload, "false") == 0)
        {
            result = false;
            return true;
        }
        return false;
    }
};
