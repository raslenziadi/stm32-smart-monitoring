#ifdef __ZEPHYR__
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(monitor_manager, LOG_LEVEL_INF);
#else
#include <stdio.h>
#define ARG_UNUSED(x) (void)(x)
#define LOG_INF(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#define LOG_WRN(...) do { printf("[WRN] "); printf(__VA_ARGS__); printf("\n"); } while (0)
#endif

#include "monitoring/monitor_manager.h"

static struct monitor_statistics stats;

void monitor_manager_init(void)
{
    stats.generated_count = 0U;
    stats.messages_received = 0U;
    stats.messages_lost = 0U;
    stats.invalid_measurements = 0U;
    stats.timeouts = 0U;
    stats.alarm_activations = 0U;
    stats.cycles = 0U;
    LOG_INF("Monitor manager initialise");
}

void monitor_manager_note_generated(uint32_t cycle)
{
    stats.generated_count++;
    stats.cycles = cycle;
}

void monitor_manager_note_message_received(uint32_t cycle)
{
    stats.messages_received++;
    stats.cycles = cycle;
}

void monitor_manager_note_message_lost(uint32_t cycle)
{
    stats.messages_lost++;
    stats.cycles = cycle;
    LOG_WRN("Message lost for cycle %u", cycle);
}

void monitor_manager_note_invalid_measurement(uint32_t cycle)
{
    stats.invalid_measurements++;
    stats.cycles = cycle;
    LOG_WRN("Invalid measurement detected at cycle %u", cycle);
}

void monitor_manager_note_timeout(void)
{
    stats.timeouts++;
    LOG_WRN("Monitor timeout event recorded");
}

void monitor_manager_note_alarm_update(enum alarm_type type,
                                      enum alarm_state previous_state,
                                      enum alarm_state new_state)
{
    ARG_UNUSED(type);
    ARG_UNUSED(previous_state);
    ARG_UNUSED(new_state);

    stats.alarm_activations++;
}

struct monitor_statistics monitor_manager_get_statistics(void)
{
    return stats;
}

void monitor_manager_print_summary(void)
{
    LOG_INF("generated=%u received=%u lost=%u invalid=%u timeouts=%u alarms=%u",
            stats.generated_count,
            stats.messages_received,
            stats.messages_lost,
            stats.invalid_measurements,
            stats.timeouts,
            stats.alarm_activations);
}
