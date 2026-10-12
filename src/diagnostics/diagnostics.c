#ifdef __ZEPHYR__
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(diagnostics, LOG_LEVEL_INF);
#else
#include <stdio.h>
#define ARG_UNUSED(x) (void)(x)
#define LOG_INF(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#define LOG_WRN(...) do { printf("[WRN] "); printf(__VA_ARGS__); printf("\n"); } while (0)
#endif

#include "diagnostics/diagnostics.h"

static struct diagnostics_snapshot snapshot;

void diagnostics_init(void)
{
    snapshot.environmental_alarms = 0U;
    snapshot.sensor_faults = 0U;
    snapshot.message_losses = 0U;
    snapshot.timeouts = 0U;
    snapshot.software_errors = 0U;
    LOG_INF("Diagnostics initialises");
}

void diagnostics_note_sensor_fault(enum sensor_type type,
                                 const struct sensor_message *message)
{
    ARG_UNUSED(message);
    if (type < SENSOR_TYPE_COUNT) {
        snapshot.sensor_faults++;
        LOG_WRN("Sensor fault type=%d", type);
    }
}

void diagnostics_note_environmental_alarm(enum alarm_type type,
                                        enum alarm_state state)
{
    if (type < ALARM_TYPE_COUNT) {
        snapshot.environmental_alarms++;
        LOG_INF("Environmental alarm type=%d state=%d", type, state);
    }
}

void diagnostics_note_message_loss(uint32_t cycle)
{
    snapshot.message_losses++;
    LOG_WRN("Message loss observed at cycle %u", cycle);
}

void diagnostics_note_timeout(void)
{
    snapshot.timeouts++;
    LOG_WRN("System timeout event");
}

void diagnostics_note_system_state(enum alarm_state state)
{
    if (state == ALARM_STATE_NORMAL) {
        LOG_INF("System state NORMAL");
    } else if (state == ALARM_STATE_WARNING) {
        LOG_INF("System state WARNING");
    } else if (state == ALARM_STATE_CRITICAL) {
        LOG_INF("System state CRITICAL");
    }
}

void diagnostics_get_snapshot(struct diagnostics_snapshot *snapshot_out)
{
    if (snapshot_out == NULL) {
        return;
    }

    *snapshot_out = snapshot;
}
