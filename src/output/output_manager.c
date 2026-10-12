#include <stdlib.h>

#ifdef __ZEPHYR__
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(output_manager, LOG_LEVEL_INF);
#else
#include <stdio.h>
#define ARG_UNUSED(x) (void)(x)
#define LOG_INF(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#endif

#include "output/output_manager.h"

void output_manager_log_measurement(const struct sensor_message *message)
{
    if (message == NULL) {
        return;
    }

    LOG_INF("cycle=%u timestamp_ms=%u temp=%d.%dC humidity=%d%% light=%d lux valid=(%d,%d,%d)",
            message->cycle,
            message->timestamp_ms,
            message->temperature_tenths / 10,
            abs(message->temperature_tenths % 10),
            message->humidity_percent,
            message->light_lux,
            message->temperature_valid,
            message->humidity_valid,
            message->light_valid);
}

void output_manager_log_system_state(enum alarm_state state,
                                    const struct monitor_statistics *stats)
{
    if (stats == NULL) {
        return;
    }

    LOG_INF("system_state=%s generated=%u received=%u lost=%u invalid=%u timeouts=%u",
            alarm_manager_state_to_string(state),
            stats->generated_count,
            stats->messages_received,
            stats->messages_lost,
            stats->invalid_measurements,
            stats->timeouts);
}

void output_manager_log_summary(void)
{
    LOG_INF("Output summary requested");
}
