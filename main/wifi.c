#include "wifi.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "wifi_manager";

wifi_manager_data_t wifi_manager_data;
EventGroupHandle_t g_wifi_event_group = NULL;

static esp_timer_handle_t g_wifi_timeout_timer = NULL;

static char *wifi_auth_mode_to_str(wifi_auth_mode_t mode) {
        switch (mode) {
        case WIFI_AUTH_OPEN:
                return "OPEN";
        case WIFI_AUTH_WEP:
                return "WEP";
        case WIFI_AUTH_WPA_PSK:
                return "WPA_PSK";
        case WIFI_AUTH_WPA2_PSK:
                return "WPA2_PSK";
        case WIFI_AUTH_WPA_WPA2_PSK:
                return "WPA_WPA2_PSK";
        case WIFI_AUTH_WPA2_ENTERPRISE:
                return "WPA2_ENTERPRISE";
        case WIFI_AUTH_WPA3_PSK:
                return "WPA3_PSK";
        case WIFI_AUTH_WPA2_WPA3_PSK:
                return "WPA2_WPA3_PSK";
        case WIFI_AUTH_WAPI_PSK:
                return "WAPI_PSK";
        case WIFI_AUTH_WPA3_EXT_PSK:
                return "WPA3_EXT_PSK";
        case WIFI_AUTH_WPA3_EXT_PSK_MIXED_MODE:
                return "WPA3_EXT_PSK_MIXED";
        case WIFI_AUTH_WPA3_ENT_192:
                return "WPA3_ENT_192";
        case WIFI_AUTH_WPA3_ENTERPRISE:
                return "WPA3_ENTERPRISE";
        case WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
                return "WPA2_WPA3_ENTERPRISE";
        case WIFI_AUTH_OWE:
                return "OWE";
        case WIFI_AUTH_MAX:
                return "MAX";
        case WIFI_AUTH_DPP:
                return "DPP";
        }
        return "auth mode error";
}

void wifi_timeout_timer_create(esp_timer_cb_t callback, const char *name,
                               uint32_t ms) {
        esp_timer_create_args_t timer_conf = {.callback = callback,
                                              .name = name};
        if (NULL != g_wifi_timeout_timer) {
                ESP_LOGE(TAG, "Create failed, timeout timer has been created");
                return;
        }
        ESP_LOGI(TAG, "start timer: %s", name);
        esp_timer_create(&timer_conf, &g_wifi_timeout_timer);
        esp_timer_start_once(g_wifi_timeout_timer, ms * 1000U);
}

void wifi_timeout_timer_delete(void) {
        if (g_wifi_timeout_timer) {
                esp_timer_stop(g_wifi_timeout_timer);
                esp_timer_delete(g_wifi_timeout_timer);
                g_wifi_timeout_timer = NULL;
        }
}

static void wifi_scan_start(void) {
        ESP_LOGI(TAG, "[ * ] wifi scan");
        xEventGroupSetBits(g_wifi_event_group, LVGL_WIFI_CONFIG_SCAN);
}

static void net_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data) {
        static int s_disconnected_handshake_count = 0;
        static int s_disconnected_unknown_count = 0;

        if (event_base == WIFI_EVENT) {
                switch (event_id) {
                case WIFI_EVENT_STA_START:

                        break;
                case WIFI_EVENT_STA_DISCONNECTED: {
                        uint8_t sta_conn_state = 0;
                        wifi_event_sta_disconnected_t *disconnected =
                            event_data;
                        switch (disconnected->reason) {
                        case WIFI_REASON_ASSOC_TOOMANY:
                                ESP_LOGW(
                                    TAG,
                                    "WIFI_REASON_ASSOC_TOOMANY Disassociated "
                                    "because AP is unable to handle all "
                                    "currently associated STAs");
                                ESP_LOGW(TAG,
                                         "The number of connected devices on "
                                         "the router exceeds the limit");

                                sta_conn_state = 1;
                                break;

                        case WIFI_REASON_MIC_FAILURE: /**< disconnected reason
                                                         code 14 */
                        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:   /**<
                                                                      diconnected
                                                                      reason code
                                                                      15 */
                        case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT: /**<
                                                                      disconnected
                                                                      reason
                                                                      code 16 */
                        case WIFI_REASON_IE_IN_4WAY_DIFFERS: /**< disconnected
                                                                reason code 17*/
                        case WIFI_REASON_HANDSHAKE_TIMEOUT:  /**< disconnected
                                                                reason code 204
                                                              */
                                ESP_LOGW(
                                    TAG,
                                    "Wi-Fi 4-way handshake failed, count: %d",
                                    s_disconnected_handshake_count);

                                if (++s_disconnected_handshake_count >= 3) {
                                        ESP_LOGW(TAG, "Router password error");
                                        sta_conn_state = 2;
                                }
                                break;

                        default:
                                if (++s_disconnected_unknown_count > 10) {
                                        ESP_LOGW(TAG, "Router password error");
                                        sta_conn_state = 3;
                                }
                                break;
                        }
                        if (sta_conn_state == 0) {
                                ESP_ERROR_CHECK(esp_wifi_connect());
                        }
                        xEventGroupSetBits(g_wifi_event_group,
                                           LVGL_WIFI_CONFIG_CONNECT_FAIL);
                        break;
                }
                case WIFI_EVENT_SCAN_DONE: {
                        wifi_event_sta_scan_done_t *scan_done_data = event_data;
                        uint16_t ap_count = scan_done_data->number;
                        if (ap_count == 0) {
                                ESP_LOGI(TAG, "[ * ] Nothing AP found");
                                break;
                        }

                        wifi_ap_record_t *wifi_ap_list =
                            (wifi_ap_record_t *)malloc(
                                sizeof(wifi_ap_record_t) * ap_count);
                        if (!wifi_ap_list) {
                                ESP_LOGE(
                                    TAG,
                                    "[ * ] malloc error, wifi_ap_list is NULL");
                                break;
                        }
                        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(
                            &ap_count, wifi_ap_list));

                        wifi_manager_data.ap_count = ap_count;
                        wifi_manager_data.ap_list = (wifi_ap_info_t *)malloc(
                            ap_count * sizeof(wifi_ap_info_t));
                        if (!wifi_manager_data.ap_list) {
                                ESP_LOGE(TAG, "[ * ] realloc error, "
                                              "ap_info_list is NULL");
                                break;
                        }
                        for (int i = 0; i < ap_count; ++i) {
                                wifi_manager_data.ap_list[i].rssi =
                                    wifi_ap_list[i].rssi;
                                wifi_manager_data.ap_list[i].authmode =
                                    wifi_ap_list[i].authmode;
                                memcpy(wifi_manager_data.ap_list[i].ssid,
                                       wifi_ap_list[i].ssid,
                                       sizeof(wifi_ap_list[i].ssid));
                        }
                        xEventGroupSetBits(g_wifi_event_group,
                                           LVGL_WIFI_CONFIG_SCAN_DONE);
                        esp_wifi_scan_stop();
                        free(wifi_ap_list);
                        break;
                }
                default:
                        break;
                }
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
                xEventGroupSetBits(g_wifi_event_group,
                                   LVGL_WIFI_CONFIG_CONNECTED);
        }
}

void initialize_wifi(void) {
        g_wifi_event_group = xEventGroupCreate();

        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        esp_event_handler_instance_t instance_any_ip;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            WIFI_EVENT, ESP_EVENT_ANY_ID, &net_event_handler, NULL,
            &instance_any_ip));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            IP_EVENT, IP_EVENT_STA_GOT_IP, &net_event_handler, NULL,
            &instance_got_ip));

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
}
