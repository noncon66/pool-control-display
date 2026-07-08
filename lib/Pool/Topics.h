#pragma once

namespace Topics
{
    namespace Status
    {
        constexpr const char* WaterTemperature = "pool/status/waterTemp";
        constexpr const char* TargetTemperature = "pool/status/targetTemp";
        constexpr const char* FilterPump        = "pool/status/filterPump";
        constexpr const char* HeatingPump       = "pool/status/heatingPump";
        constexpr const char* HeatingAllowed    = "pool/status/heatingAllowed";
        constexpr const char* Mode              = "pool/status/mode";
    }

    namespace Command
    {
        constexpr const char* SetMode          = "pool/cmd/mode";
        constexpr const char* SetTargetTemp    = "pool/cmd/targetTemp";
        constexpr const char* SetFilterPump    = "pool/cmd/filterPump";
    }

    namespace Device
    {
        constexpr const char* Availability = "pool/display/status";
        constexpr const char* Heartbeat    = "pool/display/heartbeat";
    }
}