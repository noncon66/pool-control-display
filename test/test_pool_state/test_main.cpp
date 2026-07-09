#include <unity.h>

#include "PoolState.h"

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

int main(int argc, char** argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_new_state_has_no_confirmed_values);
    RUN_TEST(test_first_confirmed_value_makes_state_available);
    RUN_TEST(test_status_becomes_stale_after_timeout);
    RUN_TEST(test_freshness_handles_millis_wraparound);
    RUN_TEST(test_heating_is_independent_from_operating_mode);
    RUN_TEST(test_manual_mode_must_be_confirmed_for_manual_control);
    return UNITY_END();
}
