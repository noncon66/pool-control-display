#include <unity.h>

#include "PoolState.h"
#include "PanelCommandState.h"
#include "PanelControlPolicy.h"
#include "PanelViewModel.h"
#include "MqttPayloadParser.h"
#include "PoolStatusUpdater.h"
#include "ScreenPowerPolicy.h"

void test_new_state_has_no_confirmed_values()
{
    PoolState state;

    TEST_ASSERT_FALSE(state.hasAnyStatus());
    TEST_ASSERT_FALSE(state.isStatusFresh(0));
}

void test_first_confirmed_value_makes_state_available()
{
    PoolState state;
    state.hasWaterTemperature = true;
    state.waterTemperature = 0.0f;
    state.lastStatusUpdateAt = 1000;

    TEST_ASSERT_TRUE(state.hasAnyStatus());
    TEST_ASSERT_TRUE(state.isStatusFresh(1000));
}

void test_status_becomes_stale_after_timeout()
{
    PoolState state;
    state.hasMode = true;
    state.lastStatusUpdateAt = 1000;

    TEST_ASSERT_TRUE(
        state.isStatusFresh(1000 + PoolState::STATUS_STALE_AFTER_MS));
    TEST_ASSERT_FALSE(
        state.isStatusFresh(1001 + PoolState::STATUS_STALE_AFTER_MS));
}

void test_freshness_handles_millis_wraparound()
{
    PoolState state;
    state.hasFilterPump = true;
    state.lastStatusUpdateAt = UINT32_MAX - 1000;

    // 2000 ms nach dem Zeitstempel, wobei uint32_t über null läuft.
    TEST_ASSERT_TRUE(state.isStatusFresh(999));
}

void test_heating_pump_is_independent_from_operating_mode()
{
    PoolState state;
    state.mode = PoolMode::Auto;
    state.hasMode = true;
    state.heatingPump = true;
    state.hasHeatingPump = true;

    TEST_ASSERT_EQUAL_UINT8(1, static_cast<uint8_t>(state.mode));
    TEST_ASSERT_TRUE(state.heatingPump);
    TEST_ASSERT_TRUE(state.hasAnyStatus());
}

void test_manual_mode_must_be_confirmed_for_manual_control()
{
    PoolState state;
    state.mode = PoolMode::Manual;

    TEST_ASSERT_FALSE(state.isManualModeConfirmed());

    state.hasMode = true;
    TEST_ASSERT_TRUE(state.isManualModeConfirmed());

    state.mode = PoolMode::Auto;
    TEST_ASSERT_FALSE(state.isManualModeConfirmed());
}

void test_invalid_float_payload_preserves_last_confirmed_value()
{
    float value = 27.5f;

    TEST_ASSERT_FALSE(MqttPayloadParser::parseFloat("not-a-number", value));
    TEST_ASSERT_EQUAL_FLOAT(27.5f, value);

    TEST_ASSERT_FALSE(MqttPayloadParser::parseFloat("29.0 extra", value));
    TEST_ASSERT_EQUAL_FLOAT(27.5f, value);

    TEST_ASSERT_FALSE(MqttPayloadParser::parseFloat("nan", value));
    TEST_ASSERT_EQUAL_FLOAT(27.5f, value);
}

void test_valid_float_payload_is_applied()
{
    float value = 27.5f;

    TEST_ASSERT_TRUE(MqttPayloadParser::parseFloat("29.5", value));
    TEST_ASSERT_EQUAL_FLOAT(29.5f, value);
}

void test_status_updater_applies_all_status_topics()
{
    PoolState state;
    PanelCommandState commands;
    const uint32_t now = 1234;

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(StatusUpdateResult::Applied),
        static_cast<uint8_t>(PoolStatusUpdater::apply(
            state, commands, Topics::Status::WaterTemperature, "27.4", now)));
    TEST_ASSERT_EQUAL_FLOAT(27.4f, state.waterTemperature);
    TEST_ASSERT_TRUE(state.hasWaterTemperature);

    PoolStatusUpdater::apply(
        state, commands, Topics::Status::TargetTemperature, "29.0", now);
    TEST_ASSERT_EQUAL_FLOAT(29.0f, state.targetTemperature);
    TEST_ASSERT_TRUE(state.hasTargetTemperature);
    TEST_ASSERT_EQUAL_UINT32(now, state.lastTargetTemperatureUpdateAt);

    PoolStatusUpdater::apply(
        state, commands, Topics::Status::FilterPump, "1", now);
    PoolStatusUpdater::apply(
        state, commands, Topics::Status::HeatingPump, "true", now);
    PoolStatusUpdater::apply(
        state, commands, Topics::Status::HeatingAllowed, "0", now);
    TEST_ASSERT_TRUE(state.filterPump);
    TEST_ASSERT_TRUE(state.hasFilterPump);
    TEST_ASSERT_EQUAL_UINT32(now, state.lastFilterPumpUpdateAt);
    TEST_ASSERT_TRUE(state.heatingPump);
    TEST_ASSERT_TRUE(state.hasHeatingPump);
    TEST_ASSERT_FALSE(state.heatingAllowed);
    TEST_ASSERT_TRUE(state.hasHeatingAllowed);

    PoolStatusUpdater::apply(
        state, commands, Topics::Status::Mode, "2", now);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(PoolMode::Manual),
        static_cast<uint8_t>(state.mode));
    TEST_ASSERT_TRUE(state.hasMode);
    TEST_ASSERT_EQUAL_UINT32(now, state.lastModeUpdateAt);
    TEST_ASSERT_EQUAL_UINT32(now, state.lastStatusUpdateAt);
}

void test_status_updater_rejects_invalid_payload_without_changes()
{
    PoolState state;
    state.waterTemperature = 26.0f;
    state.hasWaterTemperature = true;
    state.lastStatusUpdateAt = 1000;
    PanelCommandState commands;

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(StatusUpdateResult::InvalidPayload),
        static_cast<uint8_t>(PoolStatusUpdater::apply(
            state,
            commands,
            Topics::Status::WaterTemperature,
            "27.0 extra",
            2000)));
    TEST_ASSERT_EQUAL_FLOAT(26.0f, state.waterTemperature);
    TEST_ASSERT_EQUAL_UINT32(1000, state.lastStatusUpdateAt);

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(StatusUpdateResult::InvalidPayload),
        static_cast<uint8_t>(PoolStatusUpdater::apply(
            state, commands, Topics::Status::FilterPump, "yes", 2000)));
    TEST_ASSERT_FALSE(state.hasFilterPump);
    TEST_ASSERT_EQUAL_UINT32(1000, state.lastStatusUpdateAt);
}

void test_status_updater_ignores_unknown_topic()
{
    PoolState state;
    PanelCommandState commands;

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(StatusUpdateResult::UnknownTopic),
        static_cast<uint8_t>(PoolStatusUpdater::apply(
            state, commands, "pool/status/unknown", "1", 2000)));
    TEST_ASSERT_FALSE(state.hasAnyStatus());
    TEST_ASSERT_EQUAL_UINT32(0, state.lastStatusUpdateAt);
}

void test_status_updater_confirms_matching_pending_commands()
{
    PoolState state;
    PanelCommandState commands;
    commands.markModePending(PoolMode::Manual, 1000);
    commands.markTargetTemperaturePending(29.0f, 1000);
    commands.markFilterPumpPending(true, 1000);

    PoolStatusUpdater::apply(
        state, commands, Topics::Status::Mode, "2", 2000);
    PoolStatusUpdater::apply(
        state, commands, Topics::Status::TargetTemperature, "29.0", 2000);
    PoolStatusUpdater::apply(
        state, commands, Topics::Status::FilterPump, "1", 2000);

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Confirmed),
        static_cast<uint8_t>(commands.mode.progress));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Confirmed),
        static_cast<uint8_t>(commands.targetTemperature.progress));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Confirmed),
        static_cast<uint8_t>(commands.filterPump.progress));
}

void test_invalid_status_does_not_confirm_pending_command()
{
    PoolState state;
    PanelCommandState commands;
    commands.markFilterPumpPending(true, 1000);

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(StatusUpdateResult::InvalidPayload),
        static_cast<uint8_t>(PoolStatusUpdater::apply(
            state, commands, Topics::Status::FilterPump, "yes", 2000)));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Pending),
        static_cast<uint8_t>(commands.filterPump.progress));
    TEST_ASSERT_FALSE(state.hasFilterPump);
}

void test_different_valid_status_updates_state_without_confirmation()
{
    PoolState state;
    PanelCommandState commands;
    commands.markModePending(PoolMode::Manual, 1000);

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(StatusUpdateResult::Applied),
        static_cast<uint8_t>(PoolStatusUpdater::apply(
            state, commands, Topics::Status::Mode, "1", 2000)));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(PoolMode::Auto),
        static_cast<uint8_t>(state.mode));
    TEST_ASSERT_TRUE(state.hasMode);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Pending),
        static_cast<uint8_t>(commands.mode.progress));
}

void test_boolean_status_payload_is_strict()
{
    PoolState state;
    PanelCommandState commands;

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(StatusUpdateResult::InvalidPayload),
        static_cast<uint8_t>(PoolStatusUpdater::apply(
            state, commands, Topics::Status::FilterPump, "TRUE", 1000)));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(StatusUpdateResult::InvalidPayload),
        static_cast<uint8_t>(PoolStatusUpdater::apply(
            state, commands, Topics::Status::FilterPump, " true", 1000)));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(StatusUpdateResult::InvalidPayload),
        static_cast<uint8_t>(PoolStatusUpdater::apply(
            state, commands, Topics::Status::FilterPump, "1 ", 1000)));
    TEST_ASSERT_FALSE(state.hasFilterPump);
    TEST_ASSERT_EQUAL_UINT32(0, state.lastStatusUpdateAt);
}

void test_status_updater_keeps_independent_freshness_timestamps()
{
    PoolState state;
    PanelCommandState commands;

    PoolStatusUpdater::apply(
        state, commands, Topics::Status::Mode, "2", 1000);
    PoolStatusUpdater::apply(
        state, commands, Topics::Status::TargetTemperature, "28.0", 2000);
    PoolStatusUpdater::apply(
        state, commands, Topics::Status::FilterPump, "1", 3000);
    PoolStatusUpdater::apply(
        state, commands, Topics::Status::WaterTemperature, "27.0", 4000);

    TEST_ASSERT_EQUAL_UINT32(1000, state.lastModeUpdateAt);
    TEST_ASSERT_EQUAL_UINT32(2000, state.lastTargetTemperatureUpdateAt);
    TEST_ASSERT_EQUAL_UINT32(3000, state.lastFilterPumpUpdateAt);
    TEST_ASSERT_EQUAL_UINT32(4000, state.lastStatusUpdateAt);
}

void test_mode_payload_requires_exact_supported_value()
{
    PoolMode mode = PoolMode::Manual;

    TEST_ASSERT_FALSE(MqttPayloadParser::parseMode("abc", mode));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(PoolMode::Manual),
        static_cast<uint8_t>(mode));

    TEST_ASSERT_FALSE(MqttPayloadParser::parseMode("10", mode));
    TEST_ASSERT_FALSE(MqttPayloadParser::parseMode("0", mode));
    TEST_ASSERT_TRUE(MqttPayloadParser::parseMode("1", mode));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(PoolMode::Auto),
        static_cast<uint8_t>(mode));
    TEST_ASSERT_TRUE(MqttPayloadParser::parseMode("2", mode));
    TEST_ASSERT_EQUAL_UINT8(2, static_cast<uint8_t>(mode));
    TEST_ASSERT_TRUE(MqttPayloadParser::parseMode("3", mode));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(PoolMode::Off),
        static_cast<uint8_t>(mode));
}

void test_command_is_confirmed_only_by_matching_status()
{
    PanelCommandState commands;
    commands.markModePending(PoolMode::Manual, 1000);

    commands.confirmMode(PoolMode::Auto, 2000);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Pending),
        static_cast<uint8_t>(commands.mode.progress));

    commands.confirmMode(PoolMode::Manual, 2500);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Confirmed),
        static_cast<uint8_t>(commands.mode.progress));
}

void test_pending_command_times_out()
{
    PanelCommandState commands;
    commands.markFilterPumpPending(true, 1000);

    commands.updateTimeouts(
        1000 + PanelCommandState::CONFIRMATION_TIMEOUT_MS - 1);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Pending),
        static_cast<uint8_t>(commands.filterPump.progress));

    commands.updateTimeouts(
        1000 + PanelCommandState::CONFIRMATION_TIMEOUT_MS);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::TimedOut),
        static_cast<uint8_t>(commands.filterPump.progress));
}

void test_command_timeout_handles_millis_wraparound()
{
    PanelCommandState commands;
    const uint32_t startedAt = UINT32_MAX - 1000;
    commands.markModePending(PoolMode::Auto, startedAt);

    commands.updateTimeouts(
        startedAt + PanelCommandState::CONFIRMATION_TIMEOUT_MS - 1);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Pending),
        static_cast<uint8_t>(commands.mode.progress));

    commands.updateTimeouts(
        startedAt + PanelCommandState::CONFIRMATION_TIMEOUT_MS);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::TimedOut),
        static_cast<uint8_t>(commands.mode.progress));
}

void test_command_result_returns_to_idle_after_visible_period()
{
    PanelCommandState commands;
    commands.markModePending(PoolMode::Auto, 1000);
    commands.confirmMode(PoolMode::Auto, 2000);

    commands.updateTimeouts(
        2000 + PanelCommandState::RESULT_VISIBLE_MS - 1);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Confirmed),
        static_cast<uint8_t>(commands.mode.progress));

    commands.updateTimeouts(
        2000 + PanelCommandState::RESULT_VISIBLE_MS);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Idle),
        static_cast<uint8_t>(commands.mode.progress));
}

void test_target_temperature_range_and_step()
{
    TEST_ASSERT_TRUE(PanelControlPolicy::isValidTargetTemperature(20.0f));
    TEST_ASSERT_TRUE(PanelControlPolicy::isValidTargetTemperature(26.5f));
    TEST_ASSERT_TRUE(PanelControlPolicy::isValidTargetTemperature(32.0f));

    TEST_ASSERT_FALSE(PanelControlPolicy::isValidTargetTemperature(19.5f));
    TEST_ASSERT_FALSE(PanelControlPolicy::isValidTargetTemperature(32.5f));
    TEST_ASSERT_FALSE(PanelControlPolicy::isValidTargetTemperature(26.2f));
}

void test_target_temperature_adjustment_snaps_to_valid_step()
{
    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        20.5f,
        PanelControlPolicy::adjustedTargetTemperature(20.1f, 1));
    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        20.0f,
        PanelControlPolicy::adjustedTargetTemperature(20.1f, -1));
    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        29.0f,
        PanelControlPolicy::adjustedTargetTemperature(28.5f, 1));
    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        28.0f,
        PanelControlPolicy::adjustedTargetTemperature(28.5f, -1));
}

void test_target_temperature_requires_known_automatic_mode()
{
    PoolState state;
    state.hasMode = true;
    state.mode = PoolMode::Auto;
    state.lastModeUpdateAt = 1000;
    state.hasTargetTemperature = true;
    state.lastTargetTemperatureUpdateAt = 1000;
    state.lastStatusUpdateAt = 1000;

    TEST_ASSERT_TRUE(
        PanelControlPolicy::canAdjustTargetTemperature(state, 1000));

    state.hasTargetTemperature = false;
    TEST_ASSERT_FALSE(
        PanelControlPolicy::canAdjustTargetTemperature(state, 1000));
    state.hasTargetTemperature = true;

    state.mode = PoolMode::Manual;
    TEST_ASSERT_FALSE(
        PanelControlPolicy::canAdjustTargetTemperature(state, 1000));

    state.mode = PoolMode::Auto;
    TEST_ASSERT_TRUE(
        PanelControlPolicy::canAdjustTargetTemperature(
            state,
            1001 + PoolState::STATUS_STALE_AFTER_MS));
}

void test_confirmed_values_remain_usable_after_age_limit()
{
    PoolState state;
    state.hasMode = true;
    state.mode = PoolMode::Manual;
    state.lastModeUpdateAt = 1000;
    state.hasFilterPump = true;
    state.lastFilterPumpUpdateAt =
        1001 + PoolState::STATUS_STALE_AFTER_MS;

    // Eine neue Temperaturmeldung hält den allgemeinen Datenstrom aktuell,
    // darf aber den alten Manual-Modus nicht auffrischen.
    const uint32_t now = 1001 + PoolState::STATUS_STALE_AFTER_MS;
    state.hasWaterTemperature = true;
    state.lastStatusUpdateAt = now;

    TEST_ASSERT_TRUE(state.isStatusFresh(now));
    TEST_ASSERT_FALSE(state.isModeFresh(now));
    TEST_ASSERT_TRUE(
        PanelControlPolicy::canControlFilterPump(state, true, now));
}

void test_mode_selection_requires_connection_and_known_mode()
{
    PoolState state;
    state.hasMode = true;
    state.mode = PoolMode::Off;
    state.lastModeUpdateAt = 1000;

    TEST_ASSERT_TRUE(PanelControlPolicy::canSelectMode(state, true, 1000));
    TEST_ASSERT_FALSE(PanelControlPolicy::canSelectMode(state, false, 1000));
    TEST_ASSERT_TRUE(
        PanelControlPolicy::canSelectMode(
            state,
            true,
            1001 + PoolState::STATUS_STALE_AFTER_MS));

    state.hasMode = false;
    TEST_ASSERT_FALSE(
        PanelControlPolicy::canSelectMode(state, true, 1000));
}

void test_view_model_disables_controls_without_connection()
{
    PoolState state;
    state.hasMode = true;
    state.mode = PoolMode::Auto;
    state.lastModeUpdateAt = 1000;
    state.hasTargetTemperature = true;
    state.lastTargetTemperatureUpdateAt = 1000;
    state.lastStatusUpdateAt = 1000;
    PanelCommandState commands;

    const PanelViewModel view =
        PanelViewModel::create(state, commands, false, 1000);

    TEST_ASSERT_TRUE(view.showOfflineWarning);
    TEST_ASSERT_FALSE(view.modeControlEnabled);
    TEST_ASSERT_FALSE(view.targetTemperatureControlEnabled);
    TEST_ASSERT_FALSE(view.filterPumpControlEnabled);
}

void test_view_model_enables_controls_for_confirmed_mode()
{
    PoolState state;
    state.hasMode = true;
    state.mode = PoolMode::Auto;
    state.lastModeUpdateAt = 1000;
    state.hasTargetTemperature = true;
    state.lastTargetTemperatureUpdateAt = 1000;
    state.hasFilterPump = true;
    state.lastFilterPumpUpdateAt = 1000;
    state.lastStatusUpdateAt = 1000;
    PanelCommandState commands;

    PanelViewModel view =
        PanelViewModel::create(state, commands, true, 1000);
    TEST_ASSERT_TRUE(view.modeControlEnabled);
    TEST_ASSERT_TRUE(view.targetTemperatureControlEnabled);
    TEST_ASSERT_FALSE(view.filterPumpControlEnabled);

    state.mode = PoolMode::Manual;
    view = PanelViewModel::create(state, commands, true, 1000);
    TEST_ASSERT_TRUE(view.modeControlEnabled);
    TEST_ASSERT_FALSE(view.targetTemperatureControlEnabled);
    TEST_ASSERT_TRUE(view.filterPumpControlEnabled);
}

void test_view_model_exposes_command_progress()
{
    PoolState state;
    PanelCommandState commands;
    commands.markModePending(PoolMode::Auto, 1000);

    const PanelViewModel view =
        PanelViewModel::create(state, commands, true, 1000);

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(CommandProgress::Pending),
        static_cast<uint8_t>(view.modeCommand));
    TEST_ASSERT_FALSE(view.modeControlEnabled);
}

void test_screen_power_dims_and_turns_off_after_inactivity()
{
    ScreenPowerPolicy screen;
    screen.begin(1000);

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(ScreenPowerLevel::Awake),
        static_cast<uint8_t>(
            screen.update(1000 + ScreenPowerPolicy::DIM_AFTER_MS - 1)));

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(ScreenPowerLevel::Dimmed),
        static_cast<uint8_t>(
            screen.update(1000 + ScreenPowerPolicy::DIM_AFTER_MS)));
    TEST_ASSERT_EQUAL_UINT8(
        ScreenPowerPolicy::DIMMED_BRIGHTNESS_PERCENT,
        screen.brightnessPercent());

    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(ScreenPowerLevel::Off),
        static_cast<uint8_t>(
            screen.update(1000 + ScreenPowerPolicy::OFF_AFTER_MS)));
    TEST_ASSERT_EQUAL_UINT8(
        ScreenPowerPolicy::OFF_BRIGHTNESS_PERCENT,
        screen.brightnessPercent());
}

void test_wakeup_touch_is_not_forwarded_to_controls()
{
    ScreenPowerPolicy screen;
    screen.begin(1000);
    screen.update(1000 + ScreenPowerPolicy::DIM_AFTER_MS);

    TEST_ASSERT_FALSE(
        screen.handleTouch(1000 + ScreenPowerPolicy::DIM_AFTER_MS + 100));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(ScreenPowerLevel::Awake),
        static_cast<uint8_t>(screen.level()));
    TEST_ASSERT_EQUAL_UINT8(
        ScreenPowerPolicy::AWAKE_BRIGHTNESS_PERCENT,
        screen.brightnessPercent());

    TEST_ASSERT_TRUE(
        screen.handleTouch(1000 + ScreenPowerPolicy::DIM_AFTER_MS + 200));
}

void test_activity_resets_screen_power_timeout()
{
    ScreenPowerPolicy screen;
    screen.begin(1000);

    TEST_ASSERT_TRUE(screen.handleTouch(2000));
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(ScreenPowerLevel::Awake),
        static_cast<uint8_t>(
            screen.update(2000 + ScreenPowerPolicy::DIM_AFTER_MS - 1)));
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_new_state_has_no_confirmed_values);
    RUN_TEST(test_first_confirmed_value_makes_state_available);
    RUN_TEST(test_status_becomes_stale_after_timeout);
    RUN_TEST(test_freshness_handles_millis_wraparound);
    RUN_TEST(test_heating_pump_is_independent_from_operating_mode);
    RUN_TEST(test_manual_mode_must_be_confirmed_for_manual_control);
    RUN_TEST(test_invalid_float_payload_preserves_last_confirmed_value);
    RUN_TEST(test_valid_float_payload_is_applied);
    RUN_TEST(test_status_updater_applies_all_status_topics);
    RUN_TEST(test_status_updater_rejects_invalid_payload_without_changes);
    RUN_TEST(test_status_updater_ignores_unknown_topic);
    RUN_TEST(test_status_updater_confirms_matching_pending_commands);
    RUN_TEST(test_invalid_status_does_not_confirm_pending_command);
    RUN_TEST(test_different_valid_status_updates_state_without_confirmation);
    RUN_TEST(test_boolean_status_payload_is_strict);
    RUN_TEST(test_status_updater_keeps_independent_freshness_timestamps);
    RUN_TEST(test_mode_payload_requires_exact_supported_value);
    RUN_TEST(test_command_is_confirmed_only_by_matching_status);
    RUN_TEST(test_pending_command_times_out);
    RUN_TEST(test_command_timeout_handles_millis_wraparound);
    RUN_TEST(test_command_result_returns_to_idle_after_visible_period);
    RUN_TEST(test_target_temperature_range_and_step);
    RUN_TEST(test_target_temperature_adjustment_snaps_to_valid_step);
    RUN_TEST(test_target_temperature_requires_known_automatic_mode);
    RUN_TEST(test_confirmed_values_remain_usable_after_age_limit);
    RUN_TEST(test_mode_selection_requires_connection_and_known_mode);
    RUN_TEST(test_view_model_disables_controls_without_connection);
    RUN_TEST(test_view_model_enables_controls_for_confirmed_mode);
    RUN_TEST(test_view_model_exposes_command_progress);
    RUN_TEST(test_screen_power_dims_and_turns_off_after_inactivity);
    RUN_TEST(test_wakeup_touch_is_not_forwarded_to_controls);
    RUN_TEST(test_activity_resets_screen_power_timeout);
    return UNITY_END();
}
