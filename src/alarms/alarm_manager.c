#include <stdlib.h>

#ifdef __ZEPHYR__
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(alarm_manager, LOG_LEVEL_INF);
#else
#include <stdio.h>
#define ARG_UNUSED(x) (void)(x)
#define LOG_INF(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#define LOG_WRN(...) do { printf("[WRN] "); printf(__VA_ARGS__); printf("\n"); } while (0)
#define LOG_ERR(...) do { printf("[ERR] "); printf(__VA_ARGS__); printf("\n"); } while (0)
#endif

#include "alarm_manager.h"

#define TEMP_WARNING_THRESHOLD       300
#define TEMP_CRITICAL_THRESHOLD      350
#define TEMP_WARNING_CLEAR           290
#define TEMP_CRITICAL_CLEAR          340

#define HUMIDITY_WARNING_THRESHOLD   70
#define HUMIDITY_CRITICAL_THRESHOLD  85
#define HUMIDITY_WARNING_CLEAR       65
#define HUMIDITY_CRITICAL_CLEAR      80

#define LIGHT_WARNING_THRESHOLD      800
#define LIGHT_CRITICAL_THRESHOLD     1000
#define LIGHT_WARNING_CLEAR          750
#define LIGHT_CRITICAL_CLEAR         950

static enum alarm_state current_state = ALARM_STATE_NORMAL;
static uint32_t alarm_counts[ALARM_TYPE_COUNT];
static uint32_t alarm_transitions[ALARM_TYPE_COUNT];
static bool warning_active[ALARM_TYPE_COUNT];
static bool critical_active[ALARM_TYPE_COUNT];
static bool sensor_fault[ALARM_TYPE_COUNT];

const char *alarm_manager_state_to_string(enum alarm_state state)
{
    switch (state) {
    case ALARM_STATE_NORMAL:
        return "NORMAL";
    case ALARM_STATE_WARNING:
        return "WARNING";
    case ALARM_STATE_CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}

static void alarm_manager_set_channel_state(enum alarm_type type,
                                            bool warning,
                                            bool critical)
{
    bool previous_warning = warning_active[type];
    bool previous_critical = critical_active[type];

    warning_active[type] = warning;
    critical_active[type] = critical;

    if ((previous_warning != warning) || (previous_critical != critical)) {
        alarm_transitions[type]++;
        LOG_INF("Alarm transition type=%d warning=%d critical=%d",
                type, warning_active[type], critical_active[type]);
    }
}

static bool alarm_manager_get_value(const struct sensor_message *message,
                                    enum alarm_type type,
                                    int32_t *out_value)
{
    if (message == NULL || out_value == NULL) {
        return false;
    }

    switch (type) {
    case ALARM_TEMPERATURE:
        *out_value = message->temperature_tenths;
        return sensor_manager_measure_is_valid(message, SENSOR_TEMPERATURE);
    case ALARM_HUMIDITY:
        *out_value = message->humidity_percent;
        return sensor_manager_measure_is_valid(message, SENSOR_HUMIDITY);
    case ALARM_LIGHT:
        *out_value = message->light_lux;
        return sensor_manager_measure_is_valid(message, SENSOR_LIGHT);
    default:
        return false;
    }
}

static void alarm_manager_update_state(void)
{
    enum alarm_state new_state = ALARM_STATE_NORMAL;

    if (critical_active[ALARM_TEMPERATURE] ||
        critical_active[ALARM_HUMIDITY] ||
        critical_active[ALARM_LIGHT]) {
        new_state = ALARM_STATE_CRITICAL;
    } else if (warning_active[ALARM_TEMPERATURE] ||
               warning_active[ALARM_HUMIDITY] ||
               warning_active[ALARM_LIGHT]) {
        new_state = ALARM_STATE_WARNING;
    }

    if (new_state != current_state) {
        LOG_INF("Etat systeme : %s -> %s",
                alarm_manager_state_to_string(current_state),
                alarm_manager_state_to_string(new_state));
        current_state = new_state;
    }
}

void alarm_manager_init(void)
{
    current_state = ALARM_STATE_NORMAL;
    for (int i = 0; i < ALARM_TYPE_COUNT; ++i) {
        alarm_counts[i] = 0;
        alarm_transitions[i] = 0;
        warning_active[i] = false;
        critical_active[i] = false;
        sensor_fault[i] = false;
    }
    LOG_INF("Alarm manager initialise");
}

enum alarm_state alarm_manager_get_state(void)
{
    return current_state;
}

uint32_t alarm_manager_get_count(enum alarm_type type)
{
    if (type < 0 || type >= ALARM_TYPE_COUNT) {
        return 0;
    }

    return alarm_counts[type];
}

uint32_t alarm_manager_get_transition_count(enum alarm_type type)
{
    if (type < 0 || type >= ALARM_TYPE_COUNT) {
        return 0;
    }

    return alarm_transitions[type];
}

bool alarm_manager_has_sensor_fault(enum alarm_type type)
{
    if (type < 0 || type >= ALARM_TYPE_COUNT) {
        return false;
    }

    return sensor_fault[type];
}

void alarm_manager_get_summary(struct alarm_summary *summary)
{
    if (summary == NULL) {
        return;
    }

    summary->state = current_state;
    for (int i = 0; i < ALARM_TYPE_COUNT; ++i) {
        summary->activation_count[i] = alarm_counts[i];
        summary->transition_count[i] = alarm_transitions[i];
        summary->fault_active[i] = sensor_fault[i];
    }
}

void alarm_manager_process(const struct sensor_message *message)
{
    if (message == NULL) {
        LOG_ERR("Message de capteurs invalide");
        return;
    }

    int32_t temperature = 0;
    int32_t humidity = 0;
    int32_t light = 0;

    if (!alarm_manager_get_value(message, ALARM_TEMPERATURE, &temperature)) {
        if (!sensor_fault[ALARM_TEMPERATURE]) {
            LOG_WRN("Capteur temperature en defaut");
            sensor_fault[ALARM_TEMPERATURE] = true;
        }
        warning_active[ALARM_TEMPERATURE] = false;
        critical_active[ALARM_TEMPERATURE] = false;
    } else if (sensor_fault[ALARM_TEMPERATURE]) {
        sensor_fault[ALARM_TEMPERATURE] = false;
        LOG_INF("Capteur temperature valide a nouveau");
    }

    if (!alarm_manager_get_value(message, ALARM_HUMIDITY, &humidity)) {
        if (!sensor_fault[ALARM_HUMIDITY]) {
            LOG_WRN("Capteur humidite en defaut");
            sensor_fault[ALARM_HUMIDITY] = true;
        }
        warning_active[ALARM_HUMIDITY] = false;
        critical_active[ALARM_HUMIDITY] = false;
    } else if (sensor_fault[ALARM_HUMIDITY]) {
        sensor_fault[ALARM_HUMIDITY] = false;
        LOG_INF("Capteur humidite valide a nouveau");
    }

    if (!alarm_manager_get_value(message, ALARM_LIGHT, &light)) {
        if (!sensor_fault[ALARM_LIGHT]) {
            LOG_WRN("Capteur luminosite en defaut");
            sensor_fault[ALARM_LIGHT] = true;
        }
        warning_active[ALARM_LIGHT] = false;
        critical_active[ALARM_LIGHT] = false;
    } else if (sensor_fault[ALARM_LIGHT]) {
        sensor_fault[ALARM_LIGHT] = false;
        LOG_INF("Capteur luminosite valide a nouveau");
    }

    if (sensor_fault[ALARM_TEMPERATURE] || sensor_fault[ALARM_HUMIDITY] ||
        sensor_fault[ALARM_LIGHT]) {
        alarm_manager_update_state();
        return;
    }

    if (temperature >= TEMP_CRITICAL_THRESHOLD && !critical_active[ALARM_TEMPERATURE]) {
        alarm_counts[ALARM_TEMPERATURE]++;
        alarm_manager_set_channel_state(ALARM_TEMPERATURE, true, true);
        LOG_ERR("CRITIQUE temperature=%d.%d C", temperature / 10, abs(temperature % 10));
    } else if (temperature <= TEMP_CRITICAL_CLEAR && critical_active[ALARM_TEMPERATURE]) {
        alarm_manager_set_channel_state(ALARM_TEMPERATURE,
                                        temperature >= TEMP_WARNING_CLEAR,
                                        false);
        LOG_INF("Temperature retourne sous seuil critique");
    }

    if (temperature >= TEMP_WARNING_THRESHOLD && !warning_active[ALARM_TEMPERATURE] &&
        !critical_active[ALARM_TEMPERATURE]) {
        alarm_counts[ALARM_TEMPERATURE]++;
        alarm_manager_set_channel_state(ALARM_TEMPERATURE, true, false);
        LOG_WRN("ALERTE temperature=%d.%d C", temperature / 10, abs(temperature % 10));
    } else if (temperature <= TEMP_WARNING_CLEAR && warning_active[ALARM_TEMPERATURE] &&
               !critical_active[ALARM_TEMPERATURE]) {
        alarm_manager_set_channel_state(ALARM_TEMPERATURE, false, false);
        LOG_INF("Temperature redevient normale");
    }

    if (humidity >= HUMIDITY_CRITICAL_THRESHOLD && !critical_active[ALARM_HUMIDITY]) {
        alarm_counts[ALARM_HUMIDITY]++;
        alarm_manager_set_channel_state(ALARM_HUMIDITY, true, true);
        LOG_ERR("CRITIQUE humidite=%d%%", humidity);
    } else if (humidity <= HUMIDITY_CRITICAL_CLEAR && critical_active[ALARM_HUMIDITY]) {
        alarm_manager_set_channel_state(ALARM_HUMIDITY,
                                        humidity >= HUMIDITY_WARNING_CLEAR,
                                        false);
        LOG_INF("Humidite retourne sous seuil critique");
    }

    if (humidity >= HUMIDITY_WARNING_THRESHOLD && !warning_active[ALARM_HUMIDITY] &&
        !critical_active[ALARM_HUMIDITY]) {
        alarm_counts[ALARM_HUMIDITY]++;
        alarm_manager_set_channel_state(ALARM_HUMIDITY, true, false);
        LOG_WRN("ALERTE humidite=%d%%", humidity);
    } else if (humidity <= HUMIDITY_WARNING_CLEAR && warning_active[ALARM_HUMIDITY] &&
               !critical_active[ALARM_HUMIDITY]) {
        alarm_manager_set_channel_state(ALARM_HUMIDITY, false, false);
        LOG_INF("Humidite redevient normale");
    }

    if (light >= LIGHT_CRITICAL_THRESHOLD && !critical_active[ALARM_LIGHT]) {
        alarm_counts[ALARM_LIGHT]++;
        alarm_manager_set_channel_state(ALARM_LIGHT, true, true);
        LOG_ERR("CRITIQUE luminosite=%d lux", light);
    } else if (light <= LIGHT_CRITICAL_CLEAR && critical_active[ALARM_LIGHT]) {
        alarm_manager_set_channel_state(ALARM_LIGHT,
                                        light >= LIGHT_WARNING_CLEAR,
                                        false);
        LOG_INF("Luminosite retourne sous seuil critique");
    }

    if (light >= LIGHT_WARNING_THRESHOLD && !warning_active[ALARM_LIGHT] &&
        !critical_active[ALARM_LIGHT]) {
        alarm_counts[ALARM_LIGHT]++;
        alarm_manager_set_channel_state(ALARM_LIGHT, true, false);
        LOG_WRN("ALERTE luminosite=%d lux", light);
    } else if (light <= LIGHT_WARNING_CLEAR && warning_active[ALARM_LIGHT] &&
               !critical_active[ALARM_LIGHT]) {
        alarm_manager_set_channel_state(ALARM_LIGHT, false, false);
        LOG_INF("Luminosite redevient normale");
    }

    alarm_manager_update_state();
}
