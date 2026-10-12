#include <stddef.h>

#ifdef __ZEPHYR__
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sensor_simulator, LOG_LEVEL_INF);
#else
#include <stdio.h>
#define LOG_INF(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#endif

#include "simulator/sensor_simulator.h"

void sensor_simulator_config_default(struct sensor_simulator_config *config)
{
    if (config == NULL) {
        return;
    }

    config->scenario = SENSOR_SCENARIO_NORMAL;
    config->seed = 1234U;
    config->cycle_offset = 0U;
}

const char *sensor_simulator_scenario_name(enum sensor_simulation_scenario scenario)
{
    switch (scenario) {
    case SENSOR_SCENARIO_NORMAL:
        return "NORMAL";
    case SENSOR_SCENARIO_OVERHEAT:
        return "OVERHEAT";
    case SENSOR_SCENARIO_HIGH_HUMIDITY:
        return "HIGH_HUMIDITY";
    case SENSOR_SCENARIO_HIGH_LIGHT:
        return "HIGH_LIGHT";
    case SENSOR_SCENARIO_SIMULTANEOUS_ALARMS:
        return "SIMULTANEOUS_ALARMS";
    case SENSOR_SCENARIO_FAILURE:
        return "FAILURE";
    default:
        return "UNKNOWN";
    }
}

static void sensor_simulator_normal(struct sensor_message *message, uint32_t cycle)
{
    int32_t base_temp = 240 + (int32_t)((cycle + 7U) % 45U);
    int32_t base_humidity = 45 + (int32_t)((cycle * 11U) % 20U);
    int32_t base_light = 480 + (int32_t)((cycle * 17U) % 260U);

    message->temperature_tenths = base_temp;
    message->humidity_percent = base_humidity;
    message->light_lux = base_light;
}

static void sensor_simulator_overheat(struct sensor_message *message, uint32_t cycle)
{
    uint32_t phase = cycle % 60U;

    if (phase < 18U) {
        message->temperature_tenths = 240 + (int32_t)(phase * 8U);
    } else if (phase < 32U) {
        message->temperature_tenths = 360 + (int32_t)((phase - 18U) * 5U);
    } else if (phase < 46U) {
        message->temperature_tenths = 460 - (int32_t)((phase - 32U) * 7U);
    } else {
        message->temperature_tenths = 280 - (int32_t)((phase - 46U) * 3U);
    }

    message->humidity_percent = 50 + (int32_t)((cycle * 3U) % 18U);
    message->light_lux = 550 + (int32_t)((cycle * 7U) % 240U);
}

static void sensor_simulator_high_humidity(struct sensor_message *message, uint32_t cycle)
{
    uint32_t phase = cycle % 44U;

    if (phase < 22U) {
        message->humidity_percent = 55 + (int32_t)(phase * 2U);
    } else {
        message->humidity_percent = 95 - (int32_t)((phase - 22U) * 2U);
    }

    message->temperature_tenths = 240 + (int32_t)((cycle * 3U) % 50U);
    message->light_lux = 600 + (int32_t)((cycle * 13U) % 190U);
}

static void sensor_simulator_high_light(struct sensor_message *message, uint32_t cycle)
{
    uint32_t phase = cycle % 36U;

    if (phase < 20U) {
        message->light_lux = 700 + (int32_t)(phase * 18U);
    } else {
        message->light_lux = 1060 - (int32_t)((phase - 20U) * 20U);
    }

    message->temperature_tenths = 240 + (int32_t)((cycle * 5U) % 50U);
    message->humidity_percent = 50 + (int32_t)((cycle * 7U) % 20U);
}

static void sensor_simulator_simultaneous(struct sensor_message *message, uint32_t cycle)
{
    uint32_t phase = cycle % 40U;

    message->temperature_tenths = 240 + (int32_t)(phase * 6U);
    message->humidity_percent = 55 + (int32_t)(phase * 2U);
    message->light_lux = 720 + (int32_t)(phase * 18U);

    if (phase > 16U) {
        message->temperature_tenths += 150;
    }
    if (phase > 18U) {
        message->humidity_percent += 25;
        if (message->humidity_percent > 100) {
            message->humidity_percent = 100;
        }
    }
    if (phase > 14U) {
        message->light_lux += 250;
    }
}

static void sensor_simulator_failure(struct sensor_message *message, uint32_t cycle)
{
    sensor_simulator_normal(message, cycle);

    if (cycle >= 12U) {
        message->temperature_valid = false;
        message->temperature_tenths = 0;
    }
}

void sensor_simulator_generate(struct sensor_message *message,
                              uint32_t cycle,
                              const struct sensor_simulator_config *config)
{
    if (message == NULL) {
        return;
    }

    sensor_manager_init_message(message, cycle);

    enum sensor_simulation_scenario scenario = SENSOR_SCENARIO_NORMAL;
    if (config != NULL) {
        scenario = config->scenario;
    }

    switch (scenario) {
    case SENSOR_SCENARIO_NORMAL:
        sensor_simulator_normal(message, cycle + (config != NULL ? config->cycle_offset : 0U));
        break;
    case SENSOR_SCENARIO_OVERHEAT:
        sensor_simulator_overheat(message, cycle + (config != NULL ? config->cycle_offset : 0U));
        break;
    case SENSOR_SCENARIO_HIGH_HUMIDITY:
        sensor_simulator_high_humidity(message, cycle + (config != NULL ? config->cycle_offset : 0U));
        break;
    case SENSOR_SCENARIO_HIGH_LIGHT:
        sensor_simulator_high_light(message, cycle + (config != NULL ? config->cycle_offset : 0U));
        break;
    case SENSOR_SCENARIO_SIMULTANEOUS_ALARMS:
        sensor_simulator_simultaneous(message, cycle + (config != NULL ? config->cycle_offset : 0U));
        break;
    case SENSOR_SCENARIO_FAILURE:
        sensor_simulator_failure(message, cycle + (config != NULL ? config->cycle_offset : 0U));
        break;
    default:
        sensor_simulator_normal(message, cycle + (config != NULL ? config->cycle_offset : 0U));
        break;
    }

    message->temperature_valid = message->temperature_valid &&
        sensor_manager_validate_temperature(message->temperature_tenths);
    message->humidity_valid = message->humidity_valid &&
        sensor_manager_validate_humidity(message->humidity_percent);
    message->light_valid = message->light_valid &&
        sensor_manager_validate_light(message->light_lux);

    if (config != NULL && config->seed != 0U) {
        message->temperature_tenths += (int32_t)(config->seed % 7U);
    }
}
