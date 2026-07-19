#pragma once

#include <stdint.h>

#include "PanelCommandState.h"
#include "PanelControlPolicy.h"
#include "PoolState.h"

// Dieses Modell enthält genau die Informationen, die eine spätere Oberfläche
// zum Anzeigen und Freigeben ihrer Bedienelemente benötigt. Es verändert weder
// PoolState noch die Loxone-Steuerung.
struct PanelViewModel
{
    bool hasData = false;
    bool dataIsCurrent = false;
    bool showOfflineWarning = true;

    bool modeControlEnabled = false;
    bool targetTemperatureControlEnabled = false;
    bool filterPumpControlEnabled = false;

    CommandProgress modeCommand = CommandProgress::Idle;
    CommandProgress targetTemperatureCommand = CommandProgress::Idle;
    CommandProgress filterPumpCommand = CommandProgress::Idle;

    static PanelViewModel create(
        const PoolState& state,
        const PanelCommandState& commands,
        bool mqttConnected,
        uint32_t now)
    {
        PanelViewModel view;

        view.hasData = state.hasAnyStatus();
        // Retained status values are the last values confirmed by Loxone. They
        // remain usable while the MQTT transport is connected; individual
        // age limits are diagnostic only and no longer block controls.
        view.dataIsCurrent = mqttConnected && view.hasData;
        view.showOfflineWarning = !mqttConnected;

        view.modeControlEnabled =
            PanelControlPolicy::canSelectMode(state, mqttConnected, now) &&
            commands.mode.progress != CommandProgress::Pending;
        view.targetTemperatureControlEnabled =
            PanelControlPolicy::canAdjustTargetTemperature(
                state,
                mqttConnected,
                now) &&
            commands.targetTemperature.progress != CommandProgress::Pending;
        view.filterPumpControlEnabled =
            PanelControlPolicy::canControlFilterPump(
                state,
                mqttConnected,
                now) &&
            commands.filterPump.progress != CommandProgress::Pending;

        view.modeCommand = commands.mode.progress;
        view.targetTemperatureCommand = commands.targetTemperature.progress;
        view.filterPumpCommand = commands.filterPump.progress;

        return view;
    }
};
