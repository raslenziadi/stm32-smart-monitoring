#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "alarms/alarm_manager.h"
#include "simulator/sensor_simulator.h"

static void test_temperature_warning_and_critical(void)
{
    struct sensor_message message;
    sensor_manager_init_message(&message, 1U);

    alarm_manager_init();

    message.temperature_tenths = 290;
    alarm_manager_process(&message);
    assert(alarm_manager_get_state() == ALARM_STATE_NORMAL);

    message.temperature_tenths = 300;
    alarm_manager_process(&message);
    assert(alarm_manager_get_state() == ALARM_STATE_WARNING);
    assert(alarm_manager_get_count(ALARM_TEMPERATURE) == 1U);

    message.temperature_tenths = 350;
    alarm_manager_process(&message);
    assert(alarm_manager_get_state() == ALARM_STATE_CRITICAL);
    assert(alarm_manager_get_count(ALARM_TEMPERATURE) == 2U);

    message.temperature_tenths = 295;
    alarm_manager_process(&message);
    assert(alarm_manager_get_state() == ALARM_STATE_WARNING);
    assert(alarm_manager_get_count(ALARM_TEMPERATURE) == 2U);

    message.temperature_tenths = 285;
    alarm_manager_process(&message);
    assert(alarm_manager_get_state() == ALARM_STATE_NORMAL);
}

static void test_humidity_hysteresis_and_simultaneous(void)
{
    struct sensor_message message;
    sensor_manager_init_message(&message, 10U);

    alarm_manager_init();

    message.humidity_percent = 70;
    alarm_manager_process(&message);
    assert(alarm_manager_get_state() == ALARM_STATE_WARNING);

    message.humidity_percent = 86;
    alarm_manager_process(&message);
    assert(alarm_manager_get_state() == ALARM_STATE_CRITICAL);

    message.humidity_percent = 78;
    alarm_manager_process(&message);
    assert(alarm_manager_get_state() == ALARM_STATE_WARNING);

    message.temperature_tenths = 320;
    message.humidity_percent = 80;
    message.light_lux = 900;
    alarm_manager_process(&message);
    assert(alarm_manager_get_state() == ALARM_STATE_WARNING);
}

static void test_invalid_sensor_and_fault_diagnostics(void)
{
    struct sensor_message message;

    sensor_manager_init_message(&message, 15U);
    message.temperature_valid = false;
    message.humidity_percent = 50;
    message.light_lux = 500;

    alarm_manager_init();
    alarm_manager_process(&message);
    assert(alarm_manager_has_sensor_fault(ALARM_TEMPERATURE));
    assert(alarm_manager_get_state() == ALARM_STATE_NORMAL);
}

static void test_simulator_is_deterministic(void)
{
    struct sensor_message a;
    struct sensor_message b;
    struct sensor_simulator_config config = { .scenario = SENSOR_SCENARIO_OVERHEAT,
                                            .seed = 42U,
                                            .cycle_offset = 0U };

    sensor_simulator_generate(&a, 12U, &config);
    sensor_simulator_generate(&b, 12U, &config);

    assert(a.cycle == b.cycle);
    assert(a.temperature_tenths == b.temperature_tenths);
    assert(a.humidity_percent == b.humidity_percent);
    assert(a.light_lux == b.light_lux);
    assert(a.temperature_valid == b.temperature_valid);
}

int main(void)
{
    test_temperature_warning_and_critical();
    test_humidity_hysteresis_and_simultaneous();
    test_invalid_sensor_and_fault_diagnostics();
    test_simulator_is_deterministic();

    printf("All alarm and simulator tests passed.\n");
    return 0;
}
