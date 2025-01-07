#include "esp_err.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "expander.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"
#include "lv_port.h"
#include "lvgl.h"
#include "ui.h"
#include "weather_icons.h"
#include "wifi_icons.h"
#include <stdio.h>

int32_t target_temperature = 70;
int32_t indoor_temperature = 68;
int32_t outdoor_temperature = 41;
int32_t indoor_humidity = 39;
int32_t outdoor_humidity = 89;

char weather_conditions_icon[8] = WI_DAY_THUNDERSTORM;
char wifi_status_icon[8] = WIFI_ICON_2_BARS;
char current_date_string[64] = "December 30, 2024";
char current_time_string[16] = "12:00 PM";

void app_main(void) {
        ESP_ERROR_CHECK(i2c_bus_init());
        ESP_ERROR_CHECK(expander_init());
        ESP_ERROR_CHECK(lcd_init());
        ESP_ERROR_CHECK(touch_init());
        ESP_ERROR_CHECK(lvgl_init());

        lvgl_port_lock(0);
        create_ui();
        lvgl_port_unlock();

        while (1) {
                vTaskDelay(pdMS_TO_TICKS(10));
        }
}
