#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

enum sensor_type {
    SENSOR_TEMPERATURE = 0,
    SENSOR_HUMIDITY,
    SENSOR_LIGHT,
    SENSOR_TYPE_COUNT
};

struct sensor_message {
    uint32_t cycle;
    uint32_t timestamp_ms;
    int32_t temperature_tenths;
    int32_t humidity_percent;
    int32_t light_lux;
    bool temperature_valid;
    bool humidity_valid;
    bool light_valid;
};

void sensor_manager_init_message(struct sensor_message *message, uint32_t cycle);
void sensor_manager_mark_invalid(struct sensor_message *message, enum sensor_type type);
void sensor_manager_set_validity(struct sensor_message *message,
                                enum sensor_type type,
                                bool valid);
bool sensor_manager_validate_temperature(int32_t temperature_tenths);
bool sensor_manager_validate_humidity(int32_t humidity_percent);
bool sensor_manager_validate_light(int32_t light_lux);
bool sensor_manager_measure_is_valid(const struct sensor_message *message,
                                      enum sensor_type type);
bool sensor_manager_validate_message(const struct sensor_message *message);
void sensor_manager_simulate(struct sensor_message *message,
                             unsigned int cycle);

#endif /* SENSOR_MANAGER_H */
