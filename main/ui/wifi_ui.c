#include "wifi_ui.h"
#include "core/lv_obj_style_gen.h"
#include "esp_lvgl_port.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "lv_port.h"
#include "lvgl.h"
#include "misc/lv_color.h"
#include "settings_ui.h"
#include "ui.h"
#include "widgets/label/lv_label.h"
#include "wifi.h"
#include <string.h>

extern settings_screen_t settings_screen;

typedef struct {
        lv_obj_t *scr;
        lv_obj_t *heading_container;
        lv_obj_t *network_list;
        lv_obj_t *network_list_container;
        lv_obj_t *password_input;
        lv_obj_t *connect_btn;
        lv_obj_t *back_btn;
        lv_obj_t *status_label;
        lv_obj_t *refresh_btn;
        lv_obj_t *spinner;
        uint32_t last_button_press;
} wifi_screen_t;

static wifi_screen_t wifi_screen;
static const char *TAG = "WiFi UI";
TaskHandle_t wifi_event_task_handle = NULL;

static void show_spinner(void) {
        lvgl_port_lock(0);
        ESP_LOGI("Show Spinner", "Showing spinner");
        wifi_screen.spinner = lv_spinner_create(wifi_screen.scr);
        lv_obj_set_size(wifi_screen.spinner, 70, 70);
        lv_obj_center(wifi_screen.spinner);
        lvgl_port_unlock();
}

static void hide_spinner(void) {
        lvgl_port_lock(0);
        ESP_LOGI("Hide Spinner", "Hiding Spinner");
        if (wifi_screen.spinner) {
                lv_obj_delete(wifi_screen.spinner);
                wifi_screen.spinner = NULL;
        }
        lvgl_port_unlock();
}

static void wifi_scan_timeout_cb(void *timer) {
        lvgl_port_lock(0);
        ESP_LOGI("Wifi Scan Timeout Callback", "Hiding Spinner");
        hide_spinner();
        ESP_LOGI("Wifi Scan Timeout Callback",
                 "Setting head label to 'WiFi Settings'");
        lv_label_set_text(wifi_screen.status_label, "WiFi Settings");
        ESP_LOGI("Wifi Scan Timeout Callback", "Deleting Wifi Timer");
        wifi_timeout_timer_delete();
        lvgl_port_unlock();
        ESP_LOGW(TAG, "scan timeout");
}

static void start_wifi_scan(void) {
        lvgl_port_lock(0);
        ESP_LOGI("Start Wifi Scan", "Setting header label to 'Scanning...'");
        lv_label_set_text(wifi_screen.status_label, "Scanning...");
        ESP_LOGI("Start Wifi Scan", "Showing Spinner");
        show_spinner();
        lvgl_port_unlock();
        ESP_LOGI("Start Wifi Scan", "Creating Wifi Timeout Timer");
        wifi_timeout_timer_create(wifi_scan_timeout_cb, "scan_timer", 10000);
}

static void wifi_list_btn_cb(lv_event_t *e) {

        lvgl_port_lock(0);
        uint32_t now = lv_tick_get();
        if (now - wifi_screen.last_button_press < BUTTON_DEBOUNCE_MS) {
                return;
        }
        wifi_screen.last_button_press = now;
        lv_obj_t *target = lv_event_get_target_obj(e);
        ESP_LOGI(TAG, "Target text: %s",
                 lv_list_get_button_text(lv_obj_get_parent(target), target));
        lvgl_port_unlock();
}

static void wifi_scan_done(void) {
        ESP_LOGI("Wifi Scan Done", "Deleting Wifi Timeout Timer");
        wifi_timeout_timer_delete();
        lvgl_port_lock(0);
        ESP_LOGI("Wifi Scan Done", "Hiding Spinner");
        hide_spinner();
        ESP_LOGI("Wifi Scan Done", "Setting header label to 'Wifi Settings'");
        lv_label_set_text(wifi_screen.status_label, "WiFi Settings");
        lvgl_port_unlock();

        lv_obj_t *list_btn;
        if (wifi_manager_data.ap_list) {
                for (uint8_t i = 0; i < wifi_manager_data.ap_count; i++) {
                        vTaskDelay(pdMS_TO_TICKS(50));
                        lvgl_port_lock(0);
                        ESP_LOGI("Wifi Scan Done",
                                 "Adding network list button: %s",
                                 wifi_manager_data.ap_list[i].ssid);
                        list_btn = lv_list_add_button(
                            wifi_screen.network_list, LV_SYMBOL_WIFI,
                            (char *)wifi_manager_data.ap_list[i].ssid);

                        ESP_LOGI("Wifi Scan Done",
                                 "Adding network list button callback");
                        lv_obj_add_event_cb(list_btn, wifi_list_btn_cb,
                                            LV_EVENT_PRESSED, NULL);
                        lvgl_port_unlock();
                }
        }
}

static void wifi_connect_fail(void) {
        lvgl_port_lock(0);
        ESP_LOGI("Wifi Connect Fail", "Hiding Spinner");
        hide_spinner();
        lvgl_port_unlock();
        ESP_LOGW(TAG, "Connect Failed");
}

static void wifi_connect_success(void) {
        lvgl_port_lock(0);
        ESP_LOGI("Wifi Connect Success", "Hiding Spinner");
        hide_spinner();
        wifi_screen.password_input = NULL;
        lvgl_port_unlock();
}

static void wifi_event_task(void *args) {
        ESP_ERROR_CHECK(esp_task_wdt_add(wifi_event_task_handle));

        while (1) {
                ESP_LOGI("Wifi Event Task", "Running wifi event task");
                esp_task_wdt_reset();
                EventBits_t uxBits;
                uxBits = xEventGroupWaitBits(
                    g_wifi_event_group,
                    (LVGL_WIFI_CONFIG_SCAN | LVGL_WIFI_CONFIG_SCAN_DONE |
                     LVGL_WIFI_CONFIG_CONNECTED | LVGL_WIFI_CONFIG_TRY_CONNECT |
                     LVGL_WIFI_CONFIG_CONNECT_FAIL),
                    pdTRUE, pdFALSE, pdMS_TO_TICKS(1000));

                if (uxBits == 0) {
                        continue;
                }
                switch (uxBits) {
                case LVGL_WIFI_CONFIG_SCAN: {
                        ESP_LOGI("Wifi Event Task",
                                 "[ * ] Handling WIFI_CONFIG_SCAN event.");
                        wifi_scan_config_t scan_config = {.ssid = NULL,
                                                          .bssid = NULL,
                                                          .channel = 0,
                                                          .show_hidden = false};
                        ESP_ERROR_CHECK(
                            esp_wifi_scan_start(&scan_config, false));
                        vTaskDelay(pdMS_TO_TICKS(100));
                        lvgl_port_lock(0);
                        ESP_LOGI("Wifi Event Task",
                                 "[ * ] Running start_wifi_scan from "
                                 "wifi_event_task");
                        start_wifi_scan();
                        lvgl_port_unlock();
                        break;
                }
                case LVGL_WIFI_CONFIG_SCAN_DONE: {
                        ESP_LOGI("Wifi Event Task",
                                 "Handling WIFI_CONFIG_SCAN_DONE event");
                        ESP_LOGI("Wifi Event Task",
                                 "[ * ] Running refresh wifi list: %d\n",
                                 wifi_manager_data.ap_count);
                        lvgl_port_lock(0);
                        ESP_LOGI("Wifi Event Task",
                                 "[ * ] Calling wifi_scan_done");
                        wifi_scan_done();
                        lvgl_port_unlock();
                        break;
                }
                case LVGL_WIFI_CONFIG_CONNECTED:
                        ESP_LOGI("Wifi Event Handler",
                                 "Handling WIFI_CONFIG_CONNECTED event");
                        lvgl_port_lock(0);
                        ESP_LOGI("Wifi Event Task",
                                 "Calling wifi_connect_success");
                        wifi_connect_success();
                        lvgl_port_unlock();
                        break;
                case LVGL_WIFI_CONFIG_CONNECT_FAIL:
                        ESP_LOGI("Wifi Event Handler",
                                 "Handling WIFI_CONFIG_FAIL event");
                        ESP_LOGI("Wifi Event Handler",
                                 "Calling esp_wifi_disconnect");
                        esp_wifi_disconnect();
                        lvgl_port_lock(0);
                        ESP_LOGI("Wifi Event Handler",
                                 "Calling wifi_connect_fail");
                        wifi_connect_fail();
                        lvgl_port_unlock();
                        break;
                case LVGL_WIFI_CONFIG_TRY_CONNECT: {
                        wifi_config_t sta_config = {0};
                        strcpy((char *)sta_config.sta.ssid,
                               wifi_manager_data
                                   .ap_list[wifi_manager_data.current_ap]
                                   .ssid);
                        strcpy((char *)sta_config.sta.password,
                               wifi_manager_data.current_password);
                        ESP_LOGI("Wifi Event Task", "[ * ] Select SSID: %s",
                                 sta_config.sta.ssid);
                        ESP_LOGI("Wifi Event Task", "[ * ] Input Password: %s",
                                 sta_config.sta.password);
                        ESP_LOGI("Wifi Event Handler",
                                 "Attempting to connect to network");
                        esp_wifi_set_config(ESP_IF_WIFI_AP, &sta_config);
                        esp_wifi_disconnect();
                        esp_wifi_connect();
                        break;
                }
                default:
                        ESP_LOGI("Wifi Event Handler", "Unhandled event");
                        break;
                }
                esp_task_wdt_reset();
        }
}

static void wifi_start_scan(void) {
        ESP_LOGI(TAG, "[ * ] Wifi scan event");
        xEventGroupSetBits(g_wifi_event_group, LVGL_WIFI_CONFIG_SCAN);
        ESP_LOGI(TAG, "[ * ] WIFI_CONFIG_SCAN bit set");
}

static void wifi_ui_exit(lv_event_t *e) {
        ESP_LOGI("Wifi UI Exit", "Checking for wifi_event_task_handle");
        if (wifi_event_task_handle != NULL) {
                ESP_LOGI("Wifi UI Exit",
                         "Deleting wifi_event_task_handle task");
                vTaskDelete(wifi_event_task_handle);
                wifi_event_task_handle = NULL;
        }

        lvgl_port_lock(0);
        ESP_LOGI("Wifi UI Exit", "Cleaning Wifi Screen");
        lv_obj_clean(wifi_screen.scr);
        ESP_LOGI("Wifi UI Exit", "Deleting Wifi Screen");
        lv_obj_delete(wifi_screen.scr);
        ESP_LOGI("Wifi UI Exit", "Loading Settings Screen");
        lv_screen_load(settings_screen.scr);
        lvgl_port_unlock();
}

void show_wifi_ui(void) {
        ESP_LOGI(TAG, "[ * ] Setting up wifi UI");
        ESP_LOGI(TAG, "[ * ] Starting wifi event task");
        xTaskCreate(wifi_event_task, "wifi_config_task", 4096, NULL, 4,
                    &wifi_event_task_handle);
        wifi_screen.last_button_press = 0;
        lvgl_port_lock(0);

        ESP_LOGI(TAG, "Creating wifi screen");
        wifi_screen.scr = lv_obj_create(NULL);

        ESP_LOGI(TAG, "Creating wifi heading container");
        wifi_screen.heading_container = lv_obj_create(wifi_screen.scr);
        lv_obj_set_size(wifi_screen.heading_container, 800, 80);
        lv_obj_set_style_bg_color(wifi_screen.heading_container,
                                  lv_color_hex(0x1a1a1a), 0);
        lv_obj_set_style_radius(wifi_screen.heading_container, 0, 0);
        lv_obj_set_style_border_width(wifi_screen.heading_container, 0, 0);
        lv_obj_remove_flag(wifi_screen.heading_container,
                           LV_OBJ_FLAG_SCROLLABLE);

        ESP_LOGI(TAG, "Creating status label");
        wifi_screen.status_label =
            lv_label_create(wifi_screen.heading_container);
        lv_label_set_text(wifi_screen.status_label, "WiFi Settings");
        lv_obj_set_style_text_font(wifi_screen.status_label,
                                   &lv_font_montserrat_40, 0);
        lv_obj_align(wifi_screen.status_label, LV_ALIGN_TOP_MID, 0, 0);

        ESP_LOGI(TAG, "Creating refresh button");
        wifi_screen.refresh_btn =
            lv_button_create(wifi_screen.heading_container);
        lv_obj_set_size(wifi_screen.refresh_btn, 80, 50);
        lv_obj_set_style_border_width(wifi_screen.refresh_btn, 0, 0);
        lv_obj_align(wifi_screen.refresh_btn, LV_ALIGN_RIGHT_MID, -50, 0);
        lv_obj_set_style_bg_opa(wifi_screen.refresh_btn, LV_OPA_TRANSP, 0);

        ESP_LOGI(TAG, "Creating refresh button label");
        lv_obj_t *refresh_btn_label = lv_label_create(wifi_screen.refresh_btn);
        lv_label_set_text(refresh_btn_label, LV_SYMBOL_REFRESH);
        lv_obj_set_style_text_font(refresh_btn_label, &lv_font_montserrat_32,
                                   0);
        lv_obj_center(refresh_btn_label);

        ESP_LOGI(TAG, "Creating back button");
        wifi_screen.back_btn = lv_button_create(wifi_screen.heading_container);
        lv_obj_set_size(wifi_screen.back_btn, 50, 50);
        lv_obj_set_style_bg_opa(wifi_screen.back_btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(wifi_screen.back_btn, 0, 0);
        lv_obj_align(wifi_screen.back_btn, LV_ALIGN_LEFT_MID, 40, 0);
        lv_obj_add_event_cb(wifi_screen.back_btn, wifi_ui_exit,
                            LV_EVENT_PRESSED, NULL);

        ESP_LOGI(TAG, "Creating back button label");
        lv_obj_t *back_btn_label = lv_label_create(wifi_screen.back_btn);
        lv_obj_set_style_text_font(back_btn_label, &lv_font_montserrat_32, 0);
        lv_label_set_text(back_btn_label, LV_SYMBOL_LEFT);
        lv_obj_center(back_btn_label);

        ESP_LOGI(TAG, "Creating network list container");
        wifi_screen.network_list_container = lv_obj_create(wifi_screen.scr);
        lv_obj_set_size(wifi_screen.network_list_container, 750, 390);
        lv_obj_align(wifi_screen.network_list_container, LV_ALIGN_TOP_MID, 0,
                     80);

        ESP_LOGI(TAG, "Creating network list");
        wifi_screen.network_list =
            lv_list_create(wifi_screen.network_list_container);
        lv_obj_set_size(wifi_screen.network_list, 700, 380);
        lv_obj_center(wifi_screen.network_list);
        lv_obj_set_scrollbar_mode(wifi_screen.network_list,
                                  LV_SCROLLBAR_MODE_AUTO);

        ESP_LOGI(TAG, "Loading wifi screen");
        lv_screen_load(wifi_screen.scr);
        ESP_LOGI(TAG, "Starting WiFi scan");
        wifi_start_scan();
        lvgl_port_unlock();
}
