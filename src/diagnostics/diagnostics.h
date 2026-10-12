#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <stdint.h>

#include "alarms/alarm_manager.h"
#include "sensors/sensor_manager.h"

enum diagnostic_kind {
    DIAGNOSTIC_ENVIRONMENTAL_ALARM = 0,
    DIAGNOSTIC_SENSOR_FAULT,
    DIAGNOSTIC_MESSAGE_LOSS,
    DIAGNOSTIC_TIMEOUT,
    DIAGNOSTIC_SOFTWARE_ERROR,
    DIAGNOSTIC_KIND_COUNT
};

struct diagnostics_snapshot {
    uint32_t environmental_alarms;
    uint32_t sensor_faults;
    uint32_t message_losses;
    uint32_t timeouts;
    uint32_t software_errors;
};

void diagnostics_init(void);
void diagnostics_note_sensor_fault(enum sensor_type type,
                                 const struct sensor_message *message);
void diagnostics_note_environmental_alarm(enum alarm_type type,
                                        enum alarm_state state);
void diagnostics_note_message_loss(uint32_t cycle);
void diagnostics_note_timeout(void);
void diagnostics_note_system_state(enum alarm_state state);
void diagnostics_get_snapshot(struct diagnostics_snapshot *snapshot);

#endif /* DIAGNOSTICS_H */
