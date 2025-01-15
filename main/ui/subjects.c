#include "subjects.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "main.h"

static const char *TAG = "UI Subjects";

// Subjects
lv_subject_t target_temperature_subject;
lv_subject_t wifi_status_subject;
lv_subject_t indoor_temperature_subject;
lv_subject_t indoor_humidity_subject;
lv_subject_t outdoor_temperature_subject;
lv_subject_t outdoor_humidity_subject;
lv_subject_t time_subject;
lv_subject_t date_subject;
lv_subject_t weather_icon_subject;
lv_subject_t weather_conditions_subject;

void init_subjects(void) {
        lvgl_port_lock(0);
        ESP_LOGI(TAG, "Initializing subjects");
        lv_subject_init_int(&target_temperature_subject, g_target_temperature);
        lv_subject_init_int(&indoor_temperature_subject, g_indoor_temperature);
        lv_subject_init_int(&outdoor_temperature_subject,
                            g_outdoor_temperature);
        lv_subject_init_int(&indoor_humidity_subject, g_indoor_humidity);
        lv_subject_init_int(&outdoor_humidity_subject, g_outdoor_humidity);
        lv_subject_init_string(&weather_icon_subject, g_weather_conditions_icon,
                               NULL, 8, g_weather_conditions_icon);
        lv_subject_init_string(&wifi_status_subject, g_wifi_status_icon, NULL,
                               8, g_wifi_status_icon);
        lv_subject_init_string(&time_subject, g_current_time_string, NULL, 16,
                               g_current_time_string);
        lv_subject_init_string(&date_subject, g_current_date_string, NULL, 64,
                               g_current_date_string);
        lv_subject_init_string(&weather_conditions_subject,
                               g_weather_conditions_dsc, NULL, 64,
                               g_weather_conditions_dsc);
        lvgl_port_unlock();
}
