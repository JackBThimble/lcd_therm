#pragma once

#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#define WIFI_CONNECT_TIMEOUT_MS 20000
#define WIFI_SCAN_TIMEOUT_MS 10000
#define MAX_PASSWORD_LEN 64

typedef struct {
        uint8_t bssid[6];
        char ssid[MAX_SSID_LEN];
        uint8_t primary;
        wifi_second_chan_t second;
        int8_t rssi;
        wifi_auth_mode_t authmode;
} wifi_ap_info_t;

typedef struct {
        wifi_ap_info_t *ap_list;
        uint16_t ap_count;
        uint16_t current_ap;
        char current_password[MAX_PASSWORD_LEN];
        bool is_connected;
        bool is_scanning;
} wifi_manager_data_t;

extern wifi_manager_data_t wifi_manager_data;
extern EventGroupHandle_t g_wifi_event_group;
#define LVGL_WIFI_CONFIG_CONNECTED BIT0
#define LVGL_WIFI_CONFIG_SCAN BIT1
#define LVGL_WIFI_CONFIG_SCAN_DONE BIT2
#define LVGL_WIFI_CONFIG_CONNECT_FAIL BIT4
#define LVGL_WIFI_CONFIG_TRY_CONNECT BIT5
void initialize_wifi(void);
void wifi_timeout_timer_create(esp_timer_cb_t callback, const char *name,
                               uint32_t ms);
void wifi_timeout_timer_delete(void);
