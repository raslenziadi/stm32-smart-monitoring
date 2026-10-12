#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "alarms/alarm_manager.h"
#include "simulator/sensor_simulator.h"

#define TEST_CYCLES 600U

static const uint32_t test_seeds[] = { 0x1234U, 1234U };

static void test_normal_stays_below_alarm_thresholds(void)
{
    struct sensor_message message;
    struct sensor_simulator_config config = {
        .scenario = SENSOR_SCENARIO_NORMAL,
    };

    for (size_t seed_index = 0U;
         seed_index < sizeof(test_seeds) / sizeof(test_seeds[0]);
         ++seed_index) {
        config.seed = test_seeds[seed_index];
        for (uint32_t cycle = 1U; cycle <= TEST_CYCLES; ++cycle) {
            sensor_simulator_generate(&message, cycle, &config);
            assert(sensor_manager_validate_message(&message));
            assert(message.temperature_tenths < 300);
            assert(message.humidity_percent < 70);
            assert(message.light_lux < 800);

            alarm_manager_init();
            alarm_manager_process(&message);
            assert(alarm_manager_get_state() == ALARM_STATE_NORMAL);
        }
    }
}

static void test_failure_keeps_injected_temperature_invalid(void)
{
    struct sensor_message message;
    struct sensor_simulator_config config = {
        .scenario = SENSOR_SCENARIO_FAILURE,
        .seed = 0x1234U,
    };

    sensor_simulator_generate(&message, 12U, &config);

    assert(!message.temperature_valid);
    assert(message.humidity_valid);
    assert(message.light_valid);
    assert(!sensor_manager_validate_message(&message));
}

static void test_simultaneous_humidity_is_sensor_valid(void)
{
    struct sensor_message message;
    struct sensor_simulator_config config = {
        .scenario = SENSOR_SCENARIO_SIMULTANEOUS_ALARMS,
        .seed = 0x1234U,
    };

    for (uint32_t cycle = 1U; cycle <= TEST_CYCLES; ++cycle) {
        sensor_simulator_generate(&message, cycle, &config);
        assert(message.humidity_percent <= 100);
        assert(sensor_manager_validate_message(&message));
    }
}

static void test_non_target_channels_stay_below_warning(void)
{
    struct sensor_message message;
    struct sensor_simulator_config config = { .seed = 0U };

    for (size_t seed_index = 0U;
         seed_index < sizeof(test_seeds) / sizeof(test_seeds[0]);
         ++seed_index) {
        config.seed = test_seeds[seed_index];
        for (enum sensor_simulation_scenario scenario = SENSOR_SCENARIO_OVERHEAT;
             scenario <= SENSOR_SCENARIO_HIGH_LIGHT;
             ++scenario) {
            config.scenario = scenario;
            for (uint32_t cycle = 1U; cycle <= TEST_CYCLES; ++cycle) {
                sensor_simulator_generate(&message, cycle, &config);
                assert(sensor_manager_validate_message(&message));
                if (scenario == SENSOR_SCENARIO_OVERHEAT) {
                    assert(message.humidity_percent < 70);
                    assert(message.light_lux < 800);
                } else if (scenario == SENSOR_SCENARIO_HIGH_HUMIDITY) {
                    assert(message.temperature_tenths < 300);
                    assert(message.light_lux < 800);
                } else {
                    assert(message.temperature_tenths < 300);
                    assert(message.humidity_percent < 70);
                }
            }
        }
    }
}

int main(void)
{
    test_normal_stays_below_alarm_thresholds();
    test_failure_keeps_injected_temperature_invalid();
    test_simultaneous_humidity_is_sensor_valid();
    test_non_target_channels_stay_below_warning();

    puts("All simulator scenario tests passed.");
    return 0;
}