#include <unity.h>

#include "PoolState.h"
#include "PanelCommandState.h"
#include "PanelControlPolicy.h"

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

void test_heating_is_independent_from_operating_mode()
{
    PoolState state;
    state.mode = PoolMode::Auto;
    state.hasMode = true;
    state.isHeating = true;
    state.hasIsHeating = true;

    TEST_ASSERT_EQUAL_UINT8(1, static_cast<uint8_t>(state.mode));
    TEST_ASSERT_TRUE(state.isHeating);
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

void test_target_temperature_range_and_step()
{
    TEST_ASSERT_TRUE(PanelControlPolicy::isValidTargetTemperature(20.0f));
    TEST_ASSERT_TRUE(PanelControlPolicy::isValidTargetTemperature(26.5f));
    TEST_ASSERT_TRUE(PanelControlPolicy::isValidTargetTemperature(32.0f));

    TEST_ASSERT_FALSE(PanelControlPolicy::isValidTargetTemperature(19.5f));
    TEST_ASSERT_FALSE(PanelControlPolicy::isValidTargetTemperature(32.5f));
    TEST_ASSERT_FALSE(PanelControlPolicy::isValidTargetTemperature(26.2f));
}

void test_target_temperature_requires_current_automatic_mode()
{
    PoolState state;
    state.hasMode = true;
    state.mode = PoolMode::Auto;
    state.lastStatusUpdateAt = 1000;

    TEST_ASSERT_TRUE(
        PanelControlPolicy::canAdjustTargetTemperature(state, 1000));

    state.mode = PoolMode::Manual;
    TEST_ASSERT_FALSE(
        PanelControlPolicy::canAdjustTargetTemperature(state, 1000));

    state.mode = PoolMode::Auto;
    TEST_ASSERT_FALSE(
        PanelControlPolicy::canAdjustTargetTemperature(
            state,
            1001 + PoolState::STATUS_STALE_AFTER_MS));
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_new_state_has_no_confirmed_values);
    RUN_TEST(test_first_confirmed_value_makes_state_available);
    RUN_TEST(test_status_becomes_stale_after_timeout);
    RUN_TEST(test_freshness_handles_millis_wraparound);
    RUN_TEST(test_heating_is_independent_from_operating_mode);
    RUN_TEST(test_manual_mode_must_be_confirmed_for_manual_control);
    RUN_TEST(test_command_is_confirmed_only_by_matching_status);
    RUN_TEST(test_pending_command_times_out);
    RUN_TEST(test_target_temperature_range_and_step);
    RUN_TEST(test_target_temperature_requires_current_automatic_mode);
    return UNITY_END();
}
