#include "app_settings.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_task_wdt.h"
#include "expander.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"
#include "lv_port.h"
#include "lvgl.h"
#include "others/observer/lv_observer.h"
#include "rtc.h"
#include "subjects.h"
#include "temp_sensor.h"
#include "ui.h"
#include "weather_icons.h"
#include "wifi_icons.h"
#include <stdio.h>

int32_t g_target_temperature = 70;
int32_t g_indoor_temperature = 68;
int32_t g_outdoor_temperature = 41;
int32_t g_indoor_humidity = 39;
int32_t g_outdoor_humidity = 89;

char g_weather_conditions_icon[8] = WI_DAY_THUNDERSTORM;
char g_weather_conditions_dsc[64] =
    "Clear sky. Light air... And snowing like a motherfucker!!!";
char g_wifi_status_icon[8] = WIFI_ICON_2_BARS;
char g_current_date_string[64] = "December 30, 2024";
char g_current_time_string[16] = "12:00 PM";

static void update_time_task(void *pvParameters);

void app_main(void) {
        ESP_ERROR_CHECK(i2c_bus_init());
        ESP_ERROR_CHECK(expander_init());
        ESP_ERROR_CHECK(lcd_init());
        ESP_ERROR_CHECK(touch_init());
        ESP_ERROR_CHECK(lvgl_init());
        ESP_ERROR_CHECK(ext_rtc_init());
        ESP_ERROR_CHECK(temp_sensor_init());

        create_ui();

        xTaskCreate(update_time_task, "rtc_task", RTC_TASK_STACK_SIZE, NULL, 5,
                    NULL);
        while (1) {
                vTaskDelay(pdMS_TO_TICKS(10));
        }
}

static void update_time_task(void *pvParameters) {
        esp_task_wdt_add(NULL);
        struct tm timeinfo;
        static const char *months[] = {
            "January",   "February", "March",    "April",
            "May",       "June",     "July",     "August",
            "September", "October",  "November", "December",
        };

        while (1) {
                /* ESP_LOGI (TAG, */
                /*           "Resetting watchdog timer from
                 * update_time"); */
                esp_task_wdt_reset();
                /* ESP_LOGI (TAG, */
                /*           "Calling rtc_get_time from update_time");
                 */
                if (rtc_get_time(&timeinfo) == ESP_OK) {
                        int hour12 = timeinfo.tm_hour % 12;
                        if (hour12 == 0)
                                hour12 = 12;

                        snprintf(g_current_date_string,
                                 sizeof(g_current_date_string), "%s %d, %d",
                                 months[timeinfo.tm_mon], timeinfo.tm_mday,
                                 timeinfo.tm_year + 1900);
                        snprintf(g_current_time_string,
                                 sizeof(g_current_time_string), "%02d:%02d %s",
                                 hour12, timeinfo.tm_min,
                                 timeinfo.tm_hour >= 12 ? "PM" : "AM");
                        lvgl_port_lock(0);
                        lv_subject_set_pointer(&time_subject,
                                               g_current_time_string);
                        lv_subject_set_pointer(&date_subject,
                                               g_current_date_string);
                        lv_subject_notify(&time_subject);
                        lv_subject_notify(&date_subject);
                        lvgl_port_unlock();
                        esp_task_wdt_reset();
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_task_wdt_reset();
        }
}
