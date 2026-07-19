#pragma once

#include <lvgl.h>

#include "PanelViewModel.h"

class MqttManager;
struct PoolState;

// Erstellt und aktualisiert ausschließlich LVGL-Objekte. Display- und
// Touch-Hardware bleiben Aufgabe der späteren Hardwaretreiber.
class GuiManager
{
public:
    void begin(PoolState& state, MqttManager& mqtt, bool controlsEnabled = false);
    void update(uint32_t now);
    bool isInitialized() const { return _initialized; }

private:
    PoolState* _state = nullptr;
    MqttManager* _mqtt = nullptr;
    bool _initialized = false;
    bool _controlsEnabled = false;
    bool _controlStateLogged = false;
    bool _lastModeControlEnabled = false;
    bool _lastTargetControlEnabled = false;
    bool _lastFilterControlEnabled = false;
    CommandProgress _lastTargetCommand = CommandProgress::Idle;

    lv_obj_t* _waterValue = nullptr;
    lv_obj_t* _filterButton = nullptr;
    lv_obj_t* _filterValue = nullptr;
    lv_obj_t* _heatingPumpValue = nullptr;
    lv_obj_t* _heatingAllowedValue = nullptr;
    lv_obj_t* _modeButtons[3] = {nullptr, nullptr, nullptr};
    lv_obj_t* _targetValue = nullptr;
    lv_obj_t* _minusButton = nullptr;
    lv_obj_t* _plusButton = nullptr;
    lv_obj_t* _footer = nullptr;

    void createScreen();
    void applyView(const PanelViewModel& view);
    static void onModeClicked(lv_event_t* event);
    static void onTargetClicked(lv_event_t* event);
    static void onFilterPumpClicked(lv_event_t* event);
};
