#pragma once

namespace Topics
{
    // Alle MQTT-Themen stehen an einer zentralen Stelle. Bei Änderungen müssen
    // sie dadurch nicht im gesamten Programm gesucht und ersetzt werden.
    namespace Status
    {
        // Statusrichtung: Loxone veröffentlicht, das Panel empfängt.
        constexpr const char* WaterTemperature = "pool/status/waterTemp";
        constexpr const char* TargetTemperature = "pool/status/targetTemp";
        constexpr const char* FilterPump        = "pool/status/filterPump";
        constexpr const char* HeatingPump       = "pool/status/heatingPump";
        constexpr const char* HeatingAllowed    = "pool/status/heatingAllowed";
        constexpr const char* IsHeating         = "pool/status/isHeating";
        constexpr const char* Mode              = "pool/status/mode";
    }

    namespace Command
    {
        // Befehlsrichtung: Das Panel veröffentlicht einen Bedienwunsch. Loxone
        // entscheidet über die Ausführung und veröffentlicht den neuen Status.
        constexpr const char* SetMode          = "pool/cmd/mode";
        constexpr const char* SetTargetTemp    = "pool/cmd/targetTemp";
        constexpr const char* SetFilterPump    = "pool/cmd/filterPump";
    }

    namespace Device
    {
        // Technischer Zustand des Panels selbst.
        constexpr const char* Availability = "pool/display/status";
        constexpr const char* Heartbeat    = "pool/display/heartbeat";
    }
}
