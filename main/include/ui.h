#pragma once

#include "esp_err.h"
#include "lvgl.h"

typedef struct {
        lv_obj_t *scr;
        lv_obj_t *settings_button_label;
        lv_obj_t *weather_icon_label;
        lv_obj_t *weather_conditions_label;
        lv_obj_t *target_temperature_label;
        lv_obj_t *indoor_temperature_label;
        lv_obj_t *outdoor_temperature_label;
        lv_obj_t *wifi_status_label;
        lv_obj_t *indoor_humidity_label;
        lv_obj_t *outdoor_humidity_label;
        lv_obj_t *time_label;
        lv_obj_t *date_label;
        lv_obj_t *outdoor_temperature_icon_label;
        lv_obj_t *outdoor_humidity_icon_label;
        lv_obj_t *indoor_humidity_icon_label;
        lv_obj_t *indoor_temperature_icon_label;
        lv_obj_t *settings_button_shadow_label;
        lv_obj_t *weather_icon_shadow_label;
        lv_obj_t *wifi_status_shadow_label;
        lv_obj_t *target_temperature_shadow_label;
        lv_obj_t *indoor_temperature_shadow_label;
        lv_obj_t *outdoor_temperature_shadow_label;
        lv_obj_t *indoor_humidity_shadow_label;
        lv_obj_t *outdoor_humidity_shadow_label;
        lv_obj_t *time_shadow_label;
        lv_obj_t *date_shadow_label;
        lv_obj_t *outdoor_temperature_icon_shadow_label;
        lv_obj_t *outdoor_humidity_icon_shadow_label;
        lv_obj_t *weather_conditions_shadow_label;
        lv_obj_t *indoor_humidity_icon_shadow_label;
        lv_obj_t *indoor_temperature_icon_shadow_label;
        lv_obj_t *settings_button;
        lv_obj_t *target_temperature_slider;
        lv_obj_t *weather_conditions_container;
} main_screen_t;

extern main_screen_t main_screen;
esp_err_t create_ui(void);
void add_wifi_network(const char *ssid, int rssi);
