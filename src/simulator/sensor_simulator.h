#ifndef SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_H

#include <stdint.h>

#include "sensors/sensor_manager.h"

enum sensor_simulation_scenario {
    SENSOR_SCENARIO_NORMAL = 0,
    SENSOR_SCENARIO_OVERHEAT,
    SENSOR_SCENARIO_HIGH_HUMIDITY,
    SENSOR_SCENARIO_HIGH_LIGHT,
    SENSOR_SCENARIO_SIMULTANEOUS_ALARMS,
    SENSOR_SCENARIO_FAILURE,
    SENSOR_SCENARIO_COUNT
};

struct sensor_simulator_config {
    enum sensor_simulation_scenario scenario;
    uint32_t seed;
    uint32_t cycle_offset;
};

void sensor_simulator_config_default(struct sensor_simulator_config *config);
const char *sensor_simulator_scenario_name(enum sensor_simulation_scenario scenario);
void sensor_simulator_generate(struct sensor_message *message,
                              uint32_t cycle,
                              const struct sensor_simulator_config *config);

#endif /* SENSOR_SIMULATOR_H */
