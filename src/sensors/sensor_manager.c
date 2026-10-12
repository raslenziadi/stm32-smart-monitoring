#include <stdbool.h>
#include <stddef.h>

#include "sensor_manager.h"

#define TEMP_MIN_TENTHS (-400)
#define TEMP_MAX_TENTHS (1250)
#define HUMIDITY_MIN_PERCENT 0
#define HUMIDITY_MAX_PERCENT 100
#define LIGHT_MIN_LUX 0
#define LIGHT_MAX_LUX 150000

void sensor_manager_init_message(struct sensor_message *message, uint32_t cycle)
{
    if (message == NULL) {
        return;
    }

    message->cycle = cycle;
    message->timestamp_ms = cycle * 2000U;
    message->temperature_tenths = 0;
    message->humidity_percent = 0;
    message->light_lux = 0;
    message->temperature_valid = true;
    message->humidity_valid = true;
    message->light_valid = true;
}

void sensor_manager_set_validity(struct sensor_message *message,
                                enum sensor_type type,
                                bool valid)
{
    if (message == NULL) {
        return;
    }

    switch (type) {
    case SENSOR_TEMPERATURE:
        message->temperature_valid = valid;
        break;
    case SENSOR_HUMIDITY:
        message->humidity_valid = valid;
        break;
    case SENSOR_LIGHT:
        message->light_valid = valid;
        break;
    default:
        break;
    }
}

void sensor_manager_mark_invalid(struct sensor_message *message,
                                enum sensor_type type)
{
    sensor_manager_set_validity(message, type, false);
}

bool sensor_manager_validate_temperature(int32_t temperature_tenths)
{
    return temperature_tenths >= TEMP_MIN_TENTHS &&
           temperature_tenths <= TEMP_MAX_TENTHS;
}

bool sensor_manager_validate_humidity(int32_t humidity_percent)
{
    return humidity_percent >= HUMIDITY_MIN_PERCENT &&
           humidity_percent <= HUMIDITY_MAX_PERCENT;
}

bool sensor_manager_validate_light(int32_t light_lux)
{
    return light_lux >= LIGHT_MIN_LUX && light_lux <= LIGHT_MAX_LUX;
}

bool sensor_manager_measure_is_valid(const struct sensor_message *message,
                                      enum sensor_type type)
{
    if (message == NULL) {
        return false;
    }

    switch (type) {
    case SENSOR_TEMPERATURE:
        return message->temperature_valid &&
               sensor_manager_validate_temperature(message->temperature_tenths);
    case SENSOR_HUMIDITY:
        return message->humidity_valid &&
               sensor_manager_validate_humidity(message->humidity_percent);
    case SENSOR_LIGHT:
        return message->light_valid &&
               sensor_manager_validate_light(message->light_lux);
    default:
        return false;
    }
}

bool sensor_manager_validate_message(const struct sensor_message *message)
{
    if (message == NULL) {
        return false;
    }

    return sensor_manager_measure_is_valid(message, SENSOR_TEMPERATURE) &&
           sensor_manager_measure_is_valid(message, SENSOR_HUMIDITY) &&
           sensor_manager_measure_is_valid(message, SENSOR_LIGHT);
}

void sensor_manager_simulate(struct sensor_message *message,
                             unsigned int cycle)
{
    if (message == NULL) {
        return;
    }

    sensor_manager_init_message(message, cycle);

    if (cycle % 10U == 0U) {
        message->temperature_tenths = 325; /* 32.5 C */
    } else {
        message->temperature_tenths = 245 + (int32_t)(cycle % 8U) * 3;
    }

    if (cycle % 12U == 0U) {
        message->humidity_percent = 75;
    } else {
        message->humidity_percent = 55 + (int32_t)(cycle % 11U);
    }

    if (cycle % 7U == 0U) {
        message->light_lux = 850;
    } else {
        message->light_lux = 650 + ((int32_t)(cycle * 37U) % 250);
    }

    message->temperature_valid =
        sensor_manager_validate_temperature(message->temperature_tenths);
    message->humidity_valid =
        sensor_manager_validate_humidity(message->humidity_percent);
    message->light_valid = sensor_manager_validate_light(message->light_lux);
}
