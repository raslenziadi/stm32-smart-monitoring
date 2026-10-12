#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#include "sensors/sensor_manager.h"

/* Etats generaux du systeme */
enum alarm_state {
    ALARM_STATE_NORMAL = 0,
    ALARM_STATE_WARNING,
    ALARM_STATE_CRITICAL
};

/* Types d'alarmes */
enum alarm_type {
    ALARM_TEMPERATURE = 0,
    ALARM_HUMIDITY,
    ALARM_LIGHT,
    ALARM_TYPE_COUNT
};

struct alarm_summary {
    enum alarm_state state;
    uint32_t activation_count[ALARM_TYPE_COUNT];
    uint32_t transition_count[ALARM_TYPE_COUNT];
    bool fault_active[ALARM_TYPE_COUNT];
};

void alarm_manager_init(void);
void alarm_manager_process(const struct sensor_message *message);
enum alarm_state alarm_manager_get_state(void);
uint32_t alarm_manager_get_count(enum alarm_type type);
uint32_t alarm_manager_get_transition_count(enum alarm_type type);
bool alarm_manager_has_sensor_fault(enum alarm_type type);
const char *alarm_manager_state_to_string(enum alarm_state state);
void alarm_manager_get_summary(struct alarm_summary *summary);

#endif /* ALARM_MANAGER_H */
