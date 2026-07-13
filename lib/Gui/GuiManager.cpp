#include "GuiManager.h"

#include <stdio.h>

#include "MqttManager.h"
#include "PanelControlPolicy.h"
#include "PoolState.h"

namespace
{
    constexpr uint32_t COLOR_BACKGROUND = 0x0B0C0D;
    constexpr uint32_t COLOR_CARD = 0x1A1C1E;
    constexpr uint32_t COLOR_TEXT = 0xF4F4F4;
    constexpr uint32_t COLOR_MUTED = 0xA5A7AA;
    constexpr uint32_t COLOR_GREEN = 0x78BE20;
    constexpr uint32_t COLOR_DISABLED = 0x55585C;

    void styleCard(lv_obj_t* object)
    {
        lv_obj_set_style_bg_color(object, lv_color_hex(COLOR_CARD), 0);
        lv_obj_set_style_border_color(object, lv_color_hex(0x34373A), 0);
        lv_obj_set_style_border_width(object, 1, 0);
        lv_obj_set_style_radius(object, 10, 0);
        lv_obj_set_style_pad_all(object, 10, 0);
        lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    }

    lv_obj_t* createLabel(lv_obj_t* parent, const char* text, lv_color_t color)
    {
        lv_obj_t* label = lv_label_create(parent);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_color(label, color, 0);
        return label;
    }

    void setEnabled(lv_obj_t* object, bool enabled)
    {
        if (enabled) lv_obj_clear_state(object, LV_STATE_DISABLED);
        else lv_obj_add_state(object, LV_STATE_DISABLED);
    }
}

void GuiManager::begin(PoolState& state, MqttManager& mqtt)
{
    _state = &state;
    _mqtt = &mqtt;
    createScreen();
    _initialized = true;
    update(millis());
}

void GuiManager::createScreen()
{
    lv_obj_t* screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BACKGROUND), 0);
    lv_obj_set_style_text_color(screen, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_set_style_pad_all(screen, 12, 0);

    lv_obj_t* title = createLabel(screen, "POOL", lv_color_hex(COLOR_TEXT));
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 4, 0);

    lv_obj_t* connection = createLabel(screen, "WLAN  MQTT", lv_color_hex(COLOR_MUTED));
    lv_obj_align(connection, LV_ALIGN_TOP_RIGHT, -4, 8);

    lv_obj_t* temperatureCard = lv_obj_create(screen);
    styleCard(temperatureCard);
    lv_obj_set_size(temperatureCard, 456, 100);
    lv_obj_align(temperatureCard, LV_ALIGN_TOP_MID, 0, 42);
    createLabel(temperatureCard, "Wassertemperatur", lv_color_hex(COLOR_MUTED));
    _waterValue = createLabel(temperatureCard, "--,- C", lv_color_hex(COLOR_TEXT));
    lv_obj_set_style_text_font(_waterValue, &lv_font_montserrat_28, 0);
    lv_obj_align(_waterValue, LV_ALIGN_LEFT_MID, 8, 15);
    _heatingBadge = createLabel(temperatureCard, "Heizung aktiv", lv_color_hex(COLOR_GREEN));
    lv_obj_align(_heatingBadge, LV_ALIGN_RIGHT_MID, -8, 10);

    const char* statusNames[] = {"Filterpumpe", "Heizpumpe", "Heizfreigabe"};
    lv_obj_t** statusValues[] = {&_filterValue, &_heatingPumpValue, &_heatingAllowedValue};
    for (int i = 0; i < 3; ++i)
    {
        // Nur die Filterpumpenkachel ist bedienbar. Heizpumpe und Heizfreigabe
        // sind reine Informationen aus Loxone.
        lv_obj_t* card = i == 0 ? lv_btn_create(screen) : lv_obj_create(screen);
        styleCard(card);
        lv_obj_set_size(card, 145, 76);
        lv_obj_set_pos(card, 12 + i * 155, 154);
        if (i == 0)
        {
            _filterButton = card;
            lv_obj_add_event_cb(
                _filterButton,
                onFilterPumpClicked,
                LV_EVENT_CLICKED,
                this);
        }
        createLabel(card, statusNames[i], lv_color_hex(COLOR_MUTED));
        *statusValues[i] = createLabel(card, "--", lv_color_hex(COLOR_GREEN));
        lv_obj_align(*statusValues[i], LV_ALIGN_BOTTOM_LEFT, 0, 0);
    }

    lv_obj_t* modeTitle = createLabel(screen, "BETRIEBSMODUS", lv_color_hex(COLOR_MUTED));
    lv_obj_set_pos(modeTitle, 12, 240);
    const char* modeNames[] = {"AUS", "AUTOMATIK", "MANUELL"};
    for (int i = 0; i < 3; ++i)
    {
        _modeButtons[i] = lv_btn_create(screen);
        lv_obj_set_size(_modeButtons[i], 145, 52);
        lv_obj_set_pos(_modeButtons[i], 12 + i * 155, 264);
        lv_obj_set_user_data(_modeButtons[i], reinterpret_cast<void*>(i));
        lv_obj_add_event_cb(_modeButtons[i], onModeClicked, LV_EVENT_CLICKED, this);
        lv_obj_t* label = createLabel(_modeButtons[i], modeNames[i], lv_color_hex(COLOR_TEXT));
        lv_obj_center(label);
    }

    lv_obj_t* targetCard = lv_obj_create(screen);
    styleCard(targetCard);
    lv_obj_set_size(targetCard, 456, 112);
    lv_obj_set_pos(targetCard, 12, 326);
    lv_obj_t* targetTitle = createLabel(targetCard, "Solltemperatur", lv_color_hex(COLOR_MUTED));
    lv_obj_align(targetTitle, LV_ALIGN_TOP_MID, 0, 0);
    _targetValue = createLabel(targetCard, "--,- C", lv_color_hex(COLOR_TEXT));
    lv_obj_set_style_text_font(_targetValue, &lv_font_montserrat_28, 0);
    lv_obj_center(_targetValue);

    _minusButton = lv_btn_create(targetCard);
    _plusButton = lv_btn_create(targetCard);
    lv_obj_set_size(_minusButton, 58, 58);
    lv_obj_set_size(_plusButton, 58, 58);
    lv_obj_align(_minusButton, LV_ALIGN_LEFT_MID, 0, 10);
    lv_obj_align(_plusButton, LV_ALIGN_RIGHT_MID, 0, 10);
    lv_obj_set_user_data(_minusButton, reinterpret_cast<void*>(-1));
    lv_obj_set_user_data(_plusButton, reinterpret_cast<void*>(1));
    lv_obj_add_event_cb(_minusButton, onTargetClicked, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(_plusButton, onTargetClicked, LV_EVENT_CLICKED, this);
    lv_obj_center(createLabel(_minusButton, "-", lv_color_hex(COLOR_GREEN)));
    lv_obj_center(createLabel(_plusButton, "+", lv_color_hex(COLOR_GREEN)));

    _footer = createLabel(screen, "Keine Verbindung", lv_color_hex(COLOR_MUTED));
    lv_obj_align(_footer, LV_ALIGN_BOTTOM_MID, 0, 0);
}

void GuiManager::update(uint32_t now)
{
    if (!_initialized || !_state || !_mqtt) return;

    const PanelViewModel view =
        PanelViewModel::create(*_state, _mqtt->commandState(), _mqtt->isConnected(), now);
    applyView(view);

    char text[20];
    if (_state->hasWaterTemperature) snprintf(text, sizeof(text), "%.1f C", _state->waterTemperature);
    else snprintf(text, sizeof(text), "--,- C");
    lv_label_set_text(_waterValue, text);

    if (_state->hasTargetTemperature) snprintf(text, sizeof(text), "%.1f C", _state->targetTemperature);
    else snprintf(text, sizeof(text), "--,- C");
    lv_label_set_text(_targetValue, text);

    lv_label_set_text(_filterValue, _state->hasFilterPump ? (_state->filterPump ? "EIN" : "AUS") : "--");
    lv_label_set_text(_heatingPumpValue, _state->hasHeatingPump ? (_state->heatingPump ? "EIN" : "AUS") : "--");
    lv_label_set_text(_heatingAllowedValue, _state->hasHeatingAllowed ? (_state->heatingAllowed ? "JA" : "NEIN") : "--");
    if (_state->hasIsHeating && _state->isHeating) lv_obj_clear_flag(_heatingBadge, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(_heatingBadge, LV_OBJ_FLAG_HIDDEN);

    for (int i = 0; i < 3; ++i)
    {
        lv_obj_set_style_bg_color(
            _modeButtons[i],
            lv_color_hex(_state->hasMode && static_cast<int>(_state->mode) == i ? COLOR_GREEN : COLOR_CARD),
            0);
    }
}

void GuiManager::applyView(const PanelViewModel& view)
{
    for (lv_obj_t* button : _modeButtons) setEnabled(button, view.modeControlEnabled);
    setEnabled(_minusButton, view.targetTemperatureControlEnabled);
    setEnabled(_plusButton, view.targetTemperatureControlEnabled);
    setEnabled(_filterButton, view.filterPumpControlEnabled);

    const bool commandTimedOut =
        view.modeCommand == CommandProgress::TimedOut ||
        view.targetTemperatureCommand == CommandProgress::TimedOut ||
        view.filterPumpCommand == CommandProgress::TimedOut;
    const bool commandPending =
        view.modeCommand == CommandProgress::Pending ||
        view.targetTemperatureCommand == CommandProgress::Pending ||
        view.filterPumpCommand == CommandProgress::Pending;
    const bool commandConfirmed =
        view.modeCommand == CommandProgress::Confirmed ||
        view.targetTemperatureCommand == CommandProgress::Confirmed ||
        view.filterPumpCommand == CommandProgress::Confirmed;

    if (view.showOfflineWarning) lv_label_set_text(_footer, "Keine MQTT-Verbindung");
    else if (view.showStaleDataWarning) lv_label_set_text(_footer, "Loxone-Daten veraltet");
    else if (commandTimedOut) lv_label_set_text(_footer, "Keine Bestätigung von Loxone");
    else if (commandPending) lv_label_set_text(_footer, "Wird übernommen ...");
    else if (commandConfirmed) lv_label_set_text(_footer, "Übernommen");
    else lv_label_set_text(_footer, "Mit Loxone verbunden");
}

void GuiManager::onModeClicked(lv_event_t* event)
{
    auto* self = static_cast<GuiManager*>(lv_event_get_user_data(event));
    const intptr_t mode = reinterpret_cast<intptr_t>(
        lv_obj_get_user_data(lv_event_get_target(event)));
    if (self && self->_mqtt) self->_mqtt->sendMode(static_cast<uint8_t>(mode));
}

void GuiManager::onTargetClicked(lv_event_t* event)
{
    auto* self = static_cast<GuiManager*>(lv_event_get_user_data(event));
    if (!self || !self->_mqtt || !self->_state || !self->_state->hasTargetTemperature) return;

    const intptr_t direction = reinterpret_cast<intptr_t>(
        lv_obj_get_user_data(lv_event_get_target(event)));
    const float requested = self->_state->targetTemperature +
        static_cast<float>(direction) * PanelControlPolicy::TARGET_TEMPERATURE_STEP;
    self->_mqtt->sendTargetTemperature(requested);
}

void GuiManager::onFilterPumpClicked(lv_event_t* event)
{
    auto* self = static_cast<GuiManager*>(lv_event_get_user_data(event));
    if (!self || !self->_mqtt || !self->_state || !self->_state->hasFilterPump)
    {
        return;
    }

    // Das Panel sendet nur den gewünschten neuen Zustand. PoolState wird erst
    // durch die anschließende Bestätigung von Loxone geändert.
    self->_mqtt->sendFilterPump(!self->_state->filterPump);
}
