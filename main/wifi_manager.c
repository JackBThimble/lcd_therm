#include "wifi_manager.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/projdefs.h"
#include "rtc.h"
#include <stdbool.h>
#include <time.h>

#define WIFI_SSID "JaM"
#define WIFI_PASS "HooframedQMary16?"
#define MAX_RETRY 5

static const char *TAG = "wifi_manager";
static bool wifi_connected = false;
static wifi_config_data_t wifi_config_data;

static void
sync_rtc_with_ntp (void)
{
        time_t now;
        struct tm timeinfo;

        time (&now);
        localtime_r (&now, &timeinfo);

        if (timeinfo.tm_year >= (2023 - 1900))
                {
                        rtc_set_time (&timeinfo);
                        ESP_LOGI (TAG, "RTC synchronized with NTP time");
                }
}

static void
init_sntp (void)
{
        esp_sntp_config_t config
            = ESP_NETIF_SNTP_DEFAULT_CONFIG ("pool.ntp.org");
        esp_netif_sntp_init (&config);

        setenv ("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
        tzset ();

        int retry = 0;
        const int retry_count = 10;
        while (esp_netif_sntp_sync_wait (pdMS_TO_TICKS (2000))
                   == ESP_ERR_TIMEOUT
               && ++retry < retry_count)
                {
                        ESP_LOGI (
                            TAG,
                            "Waiting for system time to be set... (%d/%d)",
                            retry, retry_count);
                }
        sync_rtc_with_ntp ();
}

static void
wifi_event_handler (void *arg, esp_event_base_t event_base, int32_t event_id,
                    void *event_data)
{
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
                {
                        esp_wifi_connect ();
                }
        else if (event_base == WIFI_EVENT
                 && event_id == WIFI_EVENT_STA_DISCONNECTED)
                {
                        wifi_connected = false;
                        esp_wifi_connect ();
                }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
                {
                        wifi_connected = true;
                        init_sntp ();
                }
}

esp_err_t
wifi_manager_init (void)
{
        ESP_ERROR_CHECK (esp_netif_init ());
        ESP_ERROR_CHECK (esp_event_loop_create_default ());
        esp_netif_create_default_wifi_sta ();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT ();
        ESP_ERROR_CHECK (esp_wifi_init (&cfg));

        ESP_ERROR_CHECK (esp_event_handler_register (
            WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
        ESP_ERROR_CHECK (esp_event_handler_register (
            IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

        wifi_config_t wifi_config = {
                .sta = {
                        .ssid = WIFI_SSID,
                        .password = WIFI_PASS,
                        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                },
        };

        ESP_ERROR_CHECK (esp_wifi_set_storage (WIFI_STORAGE_RAM));
        ESP_ERROR_CHECK (esp_wifi_set_mode (WIFI_MODE_STA));
        ESP_ERROR_CHECK (esp_wifi_set_config (WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK (esp_wifi_start ());
        if (wifi_connected)
                {
                        ESP_LOGI (TAG, "Wifi connected!");
                }
        else
                {
                        ESP_LOGE (TAG, "Wifi not connected");
                }

        return ESP_OK;
}

bool
wifi_manager_is_connected (void)
{
        return wifi_connected;
}
