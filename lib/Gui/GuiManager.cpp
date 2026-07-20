#include "GuiManager.h"

#include <stdio.h>

#include "MqttManager.h"
#include "PanelControlPolicy.h"
#include "PoolState.h"

namespace
{
    constexpr uint32_t COLOR_BACKGROUND = 0x0B0C0D;
    constexpr uint32_t COLOR_CARD = 0x1A1C1E;
    constexpr uint32_t COLOR_CONTROL = 0x25282A;
    constexpr uint32_t COLOR_CONTROL_BORDER = 0x555B60;
    constexpr uint32_t COLOR_ACTIVE_CARD = 0x182414;
    constexpr uint32_t COLOR_TEXT = 0xF4F4F4;
    constexpr uint32_t COLOR_MUTED = 0xA5A7AA;
    constexpr uint32_t COLOR_GREEN = 0x78BE20;
    constexpr uint32_t COLOR_BORDER = 0x303336;
    constexpr PoolMode MODE_VALUES[] = {
        PoolMode::Off,
        PoolMode::Auto,
        PoolMode::Manual
    };

    void styleCard(lv_obj_t* object)
    {
        lv_obj_set_style_bg_color(object, lv_color_hex(COLOR_CARD), 0);
        lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(object, lv_color_hex(COLOR_BORDER), 0);
        lv_obj_set_style_border_width(object, 1, 0);
        lv_obj_set_style_radius(object, 10, 0);
        lv_obj_set_style_pad_all(object, 10, 0);
        lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    }

    void styleControl(lv_obj_t* object)
    {
        lv_obj_set_style_bg_color(object, lv_color_hex(COLOR_CONTROL), 0);
        lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(object, lv_color_hex(COLOR_CONTROL_BORDER), 0);
        lv_obj_set_style_border_width(object, 2, 0);
        lv_obj_set_style_radius(object, 10, 0);
        lv_obj_set_style_shadow_width(object, 0, 0);
        lv_obj_set_style_opa(object, 165, LV_STATE_DISABLED);
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

    const char* modeName(PoolMode mode)
    {
        switch (mode)
        {
            case PoolMode::Auto:
                return "Automatik";
            case PoolMode::Manual:
                return "Manuell";
            case PoolMode::Off:
                return "Aus";
            default:
                return "--";
        }
    }
}

void GuiManager::begin(PoolState& state, MqttManager& mqtt, bool controlsEnabled)
{
    _state = &state;
    _mqtt = &mqtt;
    _controlsEnabled = controlsEnabled;
    createScreen();
    _initialized = true;
    update(millis());
}

void GuiManager::createScreen()
{
    lv_obj_t* screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BACKGROUND), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(screen, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = createLabel(screen, "POOL", lv_color_hex(COLOR_TEXT));
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(title, 16, 4);

    _connection = createLabel(screen, "WLAN  MQTT", lv_color_hex(COLOR_MUTED));
    lv_obj_align(_connection, LV_ALIGN_TOP_RIGHT, -16, 12);

    lv_obj_t* headerLine = lv_obj_create(screen);
    lv_obj_set_size(headerLine, 456, 2);
    lv_obj_set_pos(headerLine, 12, 40);
    lv_obj_set_style_bg_color(headerLine, lv_color_hex(COLOR_GREEN), 0);
    lv_obj_set_style_bg_opa(headerLine, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(headerLine, 0, 0);
    lv_obj_set_style_pad_all(headerLine, 0, 0);
    lv_obj_clear_flag(headerLine, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* temperatureCard = lv_obj_create(screen);
    styleCard(temperatureCard);
    lv_obj_set_size(temperatureCard, 456, 54);
    lv_obj_set_pos(temperatureCard, 12, 48);
    lv_obj_set_style_pad_hor(temperatureCard, 12, 0);
    lv_obj_set_style_pad_ver(temperatureCard, 4, 0);
    lv_obj_t* waterTitle = createLabel(
        temperatureCard,
        "Wassertemperatur",
        lv_color_hex(COLOR_MUTED));
    lv_obj_align(waterTitle, LV_ALIGN_LEFT_MID, 0, 0);
    _waterValue = createLabel(temperatureCard, "--,- C", lv_color_hex(COLOR_TEXT));
    lv_obj_set_style_text_font(_waterValue, &lv_font_montserrat_28, 0);
    lv_obj_align(_waterValue, LV_ALIGN_RIGHT_MID, -32, 0);
    lv_obj_t* waterIcon = createLabel(
        temperatureCard,
        LV_SYMBOL_TINT,
        lv_color_hex(COLOR_GREEN));
    lv_obj_align(waterIcon, LV_ALIGN_RIGHT_MID, 0, 0);

    const char* statusNames[] = {"Filterpumpe", "Heizpumpe", "Heizfreigabe"};
    const char* statusIcons[] = {LV_SYMBOL_POWER, LV_SYMBOL_REFRESH, LV_SYMBOL_OK};
    lv_obj_t** statusValues[] = {&_filterValue, &_heatingPumpValue, &_heatingAllowedValue};
    const int16_t statusX[] = {12, 190, 333};
    const int16_t statusWidth[] = {170, 135, 135};
    for (int i = 0; i < 3; ++i)
    {
        // Nur die Filterpumpenkachel ist bedienbar. Heizpumpe und Heizfreigabe
        // sind reine Informationen aus Loxone.
        lv_obj_t* card = i == 0 ? lv_btn_create(screen) : lv_obj_create(screen);
        if (i == 0) styleControl(card);
        else styleCard(card);
        lv_obj_set_size(card, statusWidth[i], 82);
        lv_obj_set_pos(card, statusX[i], 110);
        lv_obj_set_style_pad_all(card, 10, 0);
        if (i == 0)
        {
            _filterButton = card;
            lv_obj_add_event_cb(
                _filterButton,
                onFilterPumpClicked,
                LV_EVENT_CLICKED,
                this);

            lv_obj_t* badge = createLabel(card, "SCHALTEN", lv_color_hex(COLOR_GREEN));
            lv_obj_set_style_bg_color(badge, lv_color_hex(COLOR_ACTIVE_CARD), 0);
            lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(badge, 8, 0);
            lv_obj_set_style_pad_hor(badge, 6, 0);
            lv_obj_set_style_pad_ver(badge, 2, 0);
            lv_obj_set_pos(badge, 32, -1);

            lv_obj_t* arrow = createLabel(card, LV_SYMBOL_RIGHT, lv_color_hex(COLOR_MUTED));
            lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, 0, 0);
        }

        lv_obj_t* icon = createLabel(card, statusIcons[i], lv_color_hex(COLOR_GREEN));
        lv_obj_set_style_text_font(icon, &lv_font_montserrat_20, 0);
        lv_obj_set_pos(icon, 0, -1);

        lv_obj_t* statusTitle = createLabel(card, statusNames[i], lv_color_hex(COLOR_MUTED));
        lv_obj_set_pos(statusTitle, 0, 27);
        *statusValues[i] = createLabel(card, "--", lv_color_hex(COLOR_GREEN));
        lv_obj_set_style_text_font(*statusValues[i], &lv_font_montserrat_20, 0);
        lv_obj_set_pos(*statusValues[i], 0, 49);
    }

    lv_obj_t* modeTitle = createLabel(screen, "BETRIEBSMODUS", lv_color_hex(COLOR_GREEN));
    lv_obj_set_style_text_font(modeTitle, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(modeTitle, 12, 198);
    _modeSummary = createLabel(screen, "--", lv_color_hex(COLOR_MUTED));
    lv_obj_align(_modeSummary, LV_ALIGN_TOP_RIGHT, -12, 202);

    const char* modeNames[] = {
        LV_SYMBOL_POWER "  AUS",
        LV_SYMBOL_REFRESH "  AUTOMATIK",
        LV_SYMBOL_SETTINGS "  MANUELL"
    };
    const int16_t modeX[] = {12, 167, 321};
    const int16_t modeWidth[] = {147, 146, 147};
    for (int i = 0; i < 3; ++i)
    {
        _modeButtons[i] = lv_btn_create(screen);
        styleControl(_modeButtons[i]);
        lv_obj_set_size(_modeButtons[i], modeWidth[i], 100);
        lv_obj_set_pos(_modeButtons[i], modeX[i], 226);
        lv_obj_set_user_data(
            _modeButtons[i],
            reinterpret_cast<void*>(
                static_cast<intptr_t>(static_cast<uint8_t>(MODE_VALUES[i]))));
        lv_obj_add_event_cb(_modeButtons[i], onModeClicked, LV_EVENT_CLICKED, this);
        lv_obj_t* label = createLabel(_modeButtons[i], modeNames[i], lv_color_hex(COLOR_TEXT));
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
        lv_obj_center(label);
    }

    lv_obj_t* targetCard = lv_obj_create(screen);
    styleCard(targetCard);
    lv_obj_set_size(targetCard, 456, 100);
    lv_obj_set_pos(targetCard, 12, 334);
    lv_obj_set_style_pad_all(targetCard, 9, 0);
    lv_obj_t* targetTitle = createLabel(targetCard, "Solltemperatur", lv_color_hex(COLOR_MUTED));
    lv_obj_align(targetTitle, LV_ALIGN_TOP_MID, 0, 1);
    _targetValue = createLabel(targetCard, "--,- C", lv_color_hex(COLOR_TEXT));
    lv_obj_set_style_text_font(_targetValue, &lv_font_montserrat_28, 0);
    lv_obj_align(_targetValue, LV_ALIGN_CENTER, 0, 13);

    _minusButton = lv_btn_create(targetCard);
    _plusButton = lv_btn_create(targetCard);
    styleControl(_minusButton);
    styleControl(_plusButton);
    lv_obj_set_size(_minusButton, 72, 78);
    lv_obj_set_size(_plusButton, 72, 78);
    lv_obj_align(_minusButton, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align(_plusButton, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_user_data(_minusButton, reinterpret_cast<void*>(-1));
    lv_obj_set_user_data(_plusButton, reinterpret_cast<void*>(1));
    // Die Touchkoordinaten streuen am unteren Displayrand beim Abheben etwas.
    // PRESSED löst genau einmal beim sicheren Erstkontakt aus, während CLICKED
    // durch die anschließende Bewegung über den Buttonrand verworfen würde.
    lv_obj_add_event_cb(_minusButton, onTargetClicked, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(_plusButton, onTargetClicked, LV_EVENT_PRESSED, this);
    lv_obj_t* minusLabel = createLabel(_minusButton, LV_SYMBOL_MINUS, lv_color_hex(COLOR_GREEN));
    lv_obj_t* plusLabel = createLabel(_plusButton, LV_SYMBOL_PLUS, lv_color_hex(COLOR_GREEN));
    lv_obj_set_style_text_font(minusLabel, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_font(plusLabel, &lv_font_montserrat_28, 0);
    lv_obj_center(minusLabel);
    lv_obj_center(plusLabel);

    _footer = createLabel(screen, "Keine Verbindung", lv_color_hex(COLOR_MUTED));
    lv_obj_align(_footer, LV_ALIGN_BOTTOM_MID, 0, -5);
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

    lv_obj_set_style_text_color(
        _connection,
        lv_color_hex(_state->mqttConnected ? COLOR_GREEN : COLOR_MUTED),
        0);
    lv_label_set_text(_modeSummary, _state->hasMode ? modeName(_state->mode) : "--");

    lv_obj_set_style_bg_color(
        _filterButton,
        lv_color_hex(_state->hasFilterPump && _state->filterPump
            ? COLOR_ACTIVE_CARD
            : COLOR_CONTROL),
        0);
    lv_obj_set_style_border_color(
        _filterButton,
        lv_color_hex(_state->hasFilterPump && _state->filterPump
            ? COLOR_GREEN
            : COLOR_CONTROL_BORDER),
        0);

    for (int i = 0; i < 3; ++i)
    {
        lv_obj_set_style_bg_color(
            _modeButtons[i],
            lv_color_hex(_state->hasMode && _state->mode == MODE_VALUES[i]
                ? COLOR_GREEN
                : COLOR_CONTROL),
            0);
        lv_obj_set_style_border_color(
            _modeButtons[i],
            lv_color_hex(_state->hasMode && _state->mode == MODE_VALUES[i]
                ? COLOR_GREEN
                : COLOR_CONTROL_BORDER),
            0);
    }
}

void GuiManager::applyView(const PanelViewModel& view)
{
    // During touch bring-up the buttons remain visually interactive while all
    // callbacks are still blocked by _controlsEnabled. Once productive control
    // is enabled, the normal offline/unknown-data policy applies again.
    const bool touchTestMode = !_controlsEnabled;
    for (lv_obj_t* button : _modeButtons)
    {
        setEnabled(button, touchTestMode || view.modeControlEnabled);
    }
    setEnabled(_minusButton, touchTestMode || view.targetTemperatureControlEnabled);
    setEnabled(_plusButton, touchTestMode || view.targetTemperatureControlEnabled);
    setEnabled(_filterButton, touchTestMode || view.filterPumpControlEnabled);

    if (!_controlStateLogged ||
        _lastModeControlEnabled != view.modeControlEnabled ||
        _lastTargetControlEnabled != view.targetTemperatureControlEnabled ||
        _lastFilterControlEnabled != view.filterPumpControlEnabled ||
        _lastTargetCommand != view.targetTemperatureCommand)
    {
        Serial.printf(
            "[GUI] controls mode=%s target=%s filter=%s targetCommand=%u\n",
            view.modeControlEnabled ? "enabled" : "disabled",
            view.targetTemperatureControlEnabled ? "enabled" : "disabled",
            view.filterPumpControlEnabled ? "enabled" : "disabled",
            static_cast<unsigned int>(view.targetTemperatureCommand));
        _controlStateLogged = true;
        _lastModeControlEnabled = view.modeControlEnabled;
        _lastTargetControlEnabled = view.targetTemperatureControlEnabled;
        _lastFilterControlEnabled = view.filterPumpControlEnabled;
        _lastTargetCommand = view.targetTemperatureCommand;
    }

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
    else if (commandTimedOut) lv_label_set_text(_footer, "Keine Bestätigung von Loxone");
    else if (commandPending) lv_label_set_text(_footer, "Wird übernommen ...");
    else if (commandConfirmed) lv_label_set_text(_footer, "Übernommen");
    else lv_label_set_text(_footer, "Mit MQTT verbunden");
}

void GuiManager::onModeClicked(lv_event_t* event)
{
    auto* self = static_cast<GuiManager*>(lv_event_get_user_data(event));
    const intptr_t mode = reinterpret_cast<intptr_t>(
        lv_obj_get_user_data(lv_event_get_target(event)));
    if (!self || !self->_mqtt) return;
    if (!self->_controlsEnabled)
    {
        Serial.printf("[GUI] mode touch=%d; MQTT control suppressed\n", static_cast<int>(mode));
        return;
    }
    self->_mqtt->sendMode(static_cast<uint8_t>(mode));
}

void GuiManager::onTargetClicked(lv_event_t* event)
{
    auto* self = static_cast<GuiManager*>(lv_event_get_user_data(event));
    if (!self || !self->_mqtt || !self->_state) return;

    const intptr_t direction = reinterpret_cast<intptr_t>(
        lv_obj_get_user_data(lv_event_get_target(event)));
    if (!self->_controlsEnabled)
    {
        Serial.printf(
            "[GUI] target touch=%s; MQTT control suppressed\n",
            direction < 0 ? "minus" : "plus");
        return;
    }
    if (!self->_state->hasTargetTemperature) return;
    const float requested = PanelControlPolicy::adjustedTargetTemperature(
        self->_state->targetTemperature,
        static_cast<int>(direction));
    self->_mqtt->sendTargetTemperature(requested);
}

void GuiManager::onFilterPumpClicked(lv_event_t* event)
{
    auto* self = static_cast<GuiManager*>(lv_event_get_user_data(event));
    if (!self || !self->_mqtt || !self->_state)
    {
        return;
    }

    // Das Panel sendet nur den gewünschten neuen Zustand. PoolState wird erst
    // durch die anschließende Bestätigung von Loxone geändert.
    if (!self->_controlsEnabled)
    {
        Serial.println("[GUI] filter touch; MQTT control suppressed");
        return;
    }
    if (!self->_state->hasFilterPump) return;
    self->_mqtt->sendFilterPump(!self->_state->filterPump);
}
