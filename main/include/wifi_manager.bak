#pragma once

#include "esp_err.h"
#include "esp_wifi.h"
#include <stdbool.h>

typedef struct
{
        uint8_t bssid[6];
        char ssid[33];
        uint8_t primary;
        wifi_second_chan_t second;
        int8_t rssi;
        wifi_auth_mode_t authmode;
} ap_info_t;

typedef struct
{
        uint16_t ap_count;
        ap_info_t *ap_info_list;
        uint16_t current_ap;
        const char *current_pwd;
} wifi_config_data_t;

esp_err_t wifi_manager_init (void);
bool wifi_manager_is_connected (void);
