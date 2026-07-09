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
    void begin(PoolState& state, MqttManager& mqtt);
    void update(uint32_t now);
    bool isInitialized() const { return _initialized; }

private:
    PoolState* _state = nullptr;
    MqttManager* _mqtt = nullptr;
    bool _initialized = false;

    lv_obj_t* _waterValue = nullptr;
    lv_obj_t* _heatingBadge = nullptr;
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
};
