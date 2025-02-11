#include "settings_ui.h"
#include "core/lv_obj_pos.h"
#include "core/lv_obj_scroll.h"
#include "core/lv_obj_style_gen.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "misc/lv_event.h"
#include "styles_ui.h"
#include "ui.h"
#include "widgets/label/lv_label.h"
#include "wifi_icons.h"
#include "wifi_ui.h"

static const char *TAG = "Settings screen";
extern settings_screen_t settings_screen;

static void wifi_settings_done_cb(void) {
        ESP_LOGI(TAG, "Returned from WiFi settings");
}

void back_button_cb(lv_event_t *e) {
        lvgl_port_lock(0);
        lv_obj_t *scr = lv_event_get_user_data(e);
        lv_obj_clean(scr);
        lv_screen_load(main_screen.scr);
        lvgl_port_unlock();
}

void wifi_btn_cb(lv_event_t *e) {
        lvgl_port_lock(0);
        show_wifi_ui();
        lvgl_port_unlock();
}

lv_obj_t *add_settings_item(lv_obj_t *parent, const char *icon,
                            const char *title, lv_event_cb_t callback,
                            void *user_data);

void create_settings_screen(settings_screen_t *ss) {
        lvgl_port_lock(0);
        ss->scr = lv_obj_create(NULL);

        ss->heading_container = lv_obj_create(ss->scr);
        lv_obj_set_size(ss->heading_container, 800, 80);
        lv_obj_set_style_bg_color(ss->heading_container, lv_color_hex(0x383838),
                                  0);
        lv_obj_set_style_radius(ss->heading_container, 0, 0);
        lv_obj_set_style_border_width(ss->heading_container, 0, 0);

        ss->header_label = lv_label_create(ss->heading_container);
        lv_obj_add_style(ss->header_label, &main_ui_style_40, 0);
        lv_label_set_text(ss->header_label, "Main Settings");
        lv_obj_center(ss->header_label);

        ss->back_button = lv_button_create(ss->heading_container);
        lv_obj_set_size(ss->back_button, 50, 50);
        lv_obj_align(ss->back_button, LV_ALIGN_LEFT_MID, 40, 0);
        lv_obj_set_style_bg_opa(ss->back_button, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(ss->back_button, 0, 0);
        lv_obj_set_ext_click_area(ss->back_button, 50);
        lv_obj_add_event_cb(ss->back_button, back_button_cb, LV_EVENT_PRESSED,
                            (void *)ss->scr);

        ss->back_button_label = lv_label_create(ss->back_button);
        lv_obj_add_style(ss->back_button_label, &main_ui_style_32, 0);
        lv_label_set_text(ss->back_button_label, LV_SYMBOL_LEFT);
        lv_obj_center(ss->back_button_label);

        ss->item_container = lv_obj_create(ss->scr);
        lv_obj_set_size(ss->item_container, 700, 320);
        lv_obj_align(ss->item_container, LV_ALIGN_CENTER, 0, 40);
        lv_obj_set_flex_flow(ss->item_container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(ss->item_container, LV_FLEX_ALIGN_SPACE_EVENLY,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_set_style_pad_all(ss->item_container, 10, 0);
        lv_obj_set_style_bg_color(ss->item_container, lv_color_hex(0x383838),
                                  0);
        lv_obj_set_style_radius(ss->item_container, 10, 0);

        ss->wifi_item = add_settings_item(ss->item_container, LV_SYMBOL_WIFI,
                                          "Wifi Settings", wifi_btn_cb, NULL);
        ss->location_item = add_settings_item(
            ss->item_container, LV_SYMBOL_GPS, "Location Settings",
            settings_select_cb, (void *)LOCATION_CB_TRIGGER);
        ss->app_item = add_settings_item(
            ss->item_container, LV_SYMBOL_SETTINGS, "Application Settings",
            settings_select_cb, (void *)APP_CB_TRIGGER);
        lvgl_port_unlock();
}

void settings_select_cb(lv_event_t *e) {
        lvgl_port_lock(0);
        lv_obj_t *target = lv_event_get_target(e);
        settings_item_trigger_type_t *trigger =
            (settings_item_trigger_type_t *)lv_obj_get_user_data(target);
        lv_obj_t *current_screen = lv_screen_active();

        switch (*trigger) {
        case WIFI_CB_TRIGGER:
                ESP_LOGI(TAG, "Target is WIFI_CB_TRIGGER");
                break;
        case LOCATION_CB_TRIGGER:
                return;
                break;
        case APP_CB_TRIGGER:
                return;
                break;
        default:
                break;
        }
        lvgl_port_unlock();
}

lv_obj_t *add_settings_item(lv_obj_t *parent, const char *icon,
                            const char *title, lv_event_cb_t callback,
                            void *user_data) {
        lvgl_port_lock(0);
        lv_obj_t *card = lv_obj_create(parent);
        lv_obj_set_size(card, 200, 260);
        lv_obj_set_style_radius(card, 10, 0);
        lv_obj_set_style_bg_color(card, lv_color_hex(0x1e1e1e), 0);
        lv_obj_set_style_shadow_width(card, 10, 0);
        lv_obj_add_event_cb(card, callback, LV_EVENT_PRESSED, NULL);
        lv_obj_set_user_data(card, user_data);
        lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);

        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_AROUND,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t *icon_label = lv_label_create(card);
        lv_label_set_text(icon_label, icon);
        lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_40, 0);

        lv_obj_t *title_label = lv_label_create(card);
        lv_obj_set_width(title_label, lv_pct(90));
        lv_label_set_long_mode(title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_label_set_text(title_label, title);
        lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0);

        lvgl_port_unlock();
        return card;
}
