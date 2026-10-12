#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "alarms/alarm_manager.h"
#include "diagnostics/diagnostics.h"
#include "monitoring/monitor_manager.h"
#include "output/output_manager.h"
#include "simulator/sensor_simulator.h"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define SENSOR_PERIOD_MS 2000
#define SENSOR_QUEUE_SIZE 8
#define SENSOR_STACK_SIZE 2048
#define SENSOR_PRIORITY 5
#define MONITOR_STACK_SIZE 2048
#define MONITOR_PRIORITY 5
#ifndef APP_SIMULATION_SCENARIO
#define APP_SIMULATION_SCENARIO SENSOR_SCENARIO_NORMAL
#endif

K_MSGQ_DEFINE(sensor_msgq, sizeof(struct sensor_message), SENSOR_QUEUE_SIZE, 4);
K_SEM_DEFINE(sensor_tick_sem, 0, 1);

static struct sensor_simulator_config app_simulator_config = {
    .scenario = APP_SIMULATION_SCENARIO,
    .seed = 0x1234U,
    .cycle_offset = 0U
};

static void sensor_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_sem_give(&sensor_tick_sem);
}

K_TIMER_DEFINE(sensor_timer, sensor_timer_expiry, NULL);
K_THREAD_STACK_DEFINE(sensor_stack, SENSOR_STACK_SIZE);
static struct k_thread sensor_thread_data;
K_THREAD_STACK_DEFINE(monitor_stack, MONITOR_STACK_SIZE);
static struct k_thread monitor_thread_data;

static void sensor_thread(void *arg1, void *arg2, void *arg3)
{
    unsigned int cycle = 0U;
    struct sensor_message message;

    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF("Sensor thread demarre. Scenario=%s",
            sensor_simulator_scenario_name(app_simulator_config.scenario));

    while (1) {
        int ret = k_sem_take(&sensor_tick_sem, K_FOREVER);

        if (ret != 0) {
            LOG_ERR("Erreur lors de l'attente du semaphore");
            continue;
        }

        cycle++;
        sensor_simulator_generate(&message, cycle, &app_simulator_config);
        monitor_manager_note_generated(cycle);
        output_manager_log_measurement(&message);

        if (!sensor_manager_validate_message(&message)) {
            monitor_manager_note_invalid_measurement(cycle);
            diagnostics_note_sensor_fault(SENSOR_TEMPERATURE, &message);
            diagnostics_note_sensor_fault(SENSOR_HUMIDITY, &message);
            diagnostics_note_sensor_fault(SENSOR_LIGHT, &message);
            continue;
        }

        ret = k_msgq_put(&sensor_msgq, &message, K_NO_WAIT);
        if (ret == 0) {
            monitor_manager_note_message_received(cycle);
            LOG_INF("Mesures envoyees dans la file k_msgq (cycle %u)", cycle);
        } else {
            monitor_manager_note_message_lost(cycle);
            LOG_WRN("File pleine : mesures du cycle %u perdues", cycle);
        }
    }
}

static void monitor_thread(void *arg1, void *arg2, void *arg3)
{
    struct sensor_message message;
    enum alarm_state previous_state = ALARM_STATE_NORMAL;

    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF("Monitor thread demarre.");

    while (1) {
        int ret = k_msgq_get(&sensor_msgq, &message, K_FOREVER);

        if (ret != 0) {
            monitor_manager_note_timeout();
            diagnostics_note_timeout();
            LOG_ERR("Erreur de reception dans la file de messages");
            continue;
        }

        LOG_INF("---- Surveillance du cycle %u ----", message.cycle);

        if (!sensor_manager_validate_message(&message)) {
            monitor_manager_note_invalid_measurement(message.cycle);
            diagnostics_note_sensor_fault(SENSOR_TEMPERATURE, &message);
            diagnostics_note_sensor_fault(SENSOR_HUMIDITY, &message);
            diagnostics_note_sensor_fault(SENSOR_LIGHT, &message);
            continue;
        }

        previous_state = alarm_manager_get_state();
        alarm_manager_process(&message);

        if (previous_state != alarm_manager_get_state()) {
            monitor_manager_note_alarm_update(ALARM_TEMPERATURE,
                                              previous_state,
                                              alarm_manager_get_state());
        }

        diagnostics_note_system_state(alarm_manager_get_state());

        struct monitor_statistics stats = monitor_manager_get_statistics();
        output_manager_log_system_state(alarm_manager_get_state(), &stats);

        LOG_INF("Etat systeme: %s | Compteurs: temp=%u humidite=%u luminosite=%u",
                alarm_manager_state_to_string(alarm_manager_get_state()),
                alarm_manager_get_count(ALARM_TEMPERATURE),
                alarm_manager_get_count(ALARM_HUMIDITY),
                alarm_manager_get_count(ALARM_LIGHT));
    }
}

int main(void)
{
    LOG_INF("====================================");
    LOG_INF("STM32 SMART MONITORING SYSTEM");
    LOG_INF("Zephyr RTOS - Architecture modulaire");
    LOG_INF("Communication inter-threads : k_msgq");
    LOG_INF("Synchronisation : k_timer + k_sem");
    LOG_INF("====================================");

    alarm_manager_init();
    monitor_manager_init();
    diagnostics_init();

    k_tid_t sensor_tid = k_thread_create(&sensor_thread_data,
                                         sensor_stack,
                                         K_THREAD_STACK_SIZEOF(sensor_stack),
                                         sensor_thread,
                                         NULL, NULL, NULL,
                                         SENSOR_PRIORITY,
                                         0,
                                         K_NO_WAIT);
    if (sensor_tid == NULL) {
        LOG_ERR("Echec de creation du sensor thread");
        return -1;
    }
    k_thread_name_set(&sensor_thread_data, "sensor_thread");

    k_tid_t monitor_tid = k_thread_create(&monitor_thread_data,
                                         monitor_stack,
                                         K_THREAD_STACK_SIZEOF(monitor_stack),
                                         monitor_thread,
                                         NULL, NULL, NULL,
                                         MONITOR_PRIORITY,
                                         0,
                                         K_NO_WAIT);
    if (monitor_tid == NULL) {
        LOG_ERR("Echec de creation du monitor thread");
        k_thread_abort(&sensor_thread_data);
        return -1;
    }
    k_thread_name_set(&monitor_thread_data, "monitor_thread");

    k_timer_start(&sensor_timer, K_MSEC(SENSOR_PERIOD_MS), K_MSEC(SENSOR_PERIOD_MS));

    LOG_INF("Timer d'acquisition demarre. Periode=%d ms", SENSOR_PERIOD_MS);
    LOG_INF("Systeme pret.");

    return 0;
}
