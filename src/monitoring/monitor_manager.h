#ifndef MONITOR_MANAGER_H
#define MONITOR_MANAGER_H

#include <stdint.h>

#include "alarms/alarm_manager.h"

struct monitor_statistics {
    uint32_t generated_count;
    uint32_t messages_received;
    uint32_t messages_lost;
    uint32_t invalid_measurements;
    uint32_t timeouts;
    uint32_t alarm_activations;
    uint32_t cycles;
};

void monitor_manager_init(void);
void monitor_manager_note_generated(uint32_t cycle);
void monitor_manager_note_message_received(uint32_t cycle);
void monitor_manager_note_message_lost(uint32_t cycle);
void monitor_manager_note_invalid_measurement(uint32_t cycle);
void monitor_manager_note_timeout(void);
void monitor_manager_note_alarm_update(enum alarm_type type,
                                      enum alarm_state previous_state,
                                      enum alarm_state new_state);
struct monitor_statistics monitor_manager_get_statistics(void);
void monitor_manager_print_summary(void);

#endif /* MONITOR_MANAGER_H */
