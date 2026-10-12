#ifndef OUTPUT_MANAGER_H
#define OUTPUT_MANAGER_H

#include "monitoring/monitor_manager.h"
#include "sensors/sensor_manager.h"

void output_manager_log_measurement(const struct sensor_message *message);
void output_manager_log_system_state(enum alarm_state state,
                                    const struct monitor_statistics *stats);
void output_manager_log_summary(void);

#endif /* OUTPUT_MANAGER_H */
