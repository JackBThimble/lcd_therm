#include "ui.h"
#include "core/lv_obj_pos.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "fonts/lcd_therm_fonts.h"
#include "freertos/task.h"
#include "images/lcd_therm_images.h"
#include "main.h"
#include "misc/lv_area.h"
#include "misc/lv_color.h"
#include "misc/lv_style.h"
#include "settings_ui.h"
#include "slider_ui.h"
#include "styles_ui.h"
#include "subjects_ui.h"
#include "weather_icons.h"
#include "widgets/label/lv_label.h"
#include <string.h>

static const char *TAG = "UI";
lv_mem_monitor_t mem;

void create_settings_button(lv_obj_t *scr, lv_align_t align, int16_t offset_x,
                            int16_t offset_y);
void create_wifi_status_label(lv_obj_t *scr, lv_align_t align, int16_t offset_x,
                              int16_t offset_y);
void create_target_temperature_label(lv_obj_t *, lv_align_t align,
                                     int16_t offset_x, int16_t offset_y);
void create_indoor_temperature_label(lv_obj_t *, lv_align_t align,
                                     int16_t offset_x, int16_t offset_y);
void create_outdoor_temperature_label(lv_obj_t *, lv_align_t align,
                                      int16_t offset_x, int16_t offset_y);
void create_indoor_humidity_label(lv_obj_t *, lv_align_t align,
                                  int16_t offset_x, int16_t offset_y);
void create_outdoor_humidity_label(lv_obj_t *, lv_align_t align,
                                   int16_t offset_x, int16_t offset_y);
void create_weather_icon_label(lv_obj_t *, lv_align_t align, int16_t offset_x,
                               int16_t offset_y);
void create_current_time_label(lv_obj_t *, lv_align_t align, int16_t offset_x,
                               int16_t offset_y);
void create_current_date_label(lv_obj_t *, lv_align_t align, int16_t offset_x,
                               int16_t offset_y);
void main_screen_settings_button_cb(lv_event_t *e);
void create_main_screen();

// Screens
main_screen_t main_screen;
settings_screen_t settings_screen;

esp_err_t create_ui(void) {
        lvgl_port_lock(0);

        lv_mem_monitor(&mem);
        init_styles();
        init_subjects();

        create_main_screen();

        lv_screen_load(main_screen.scr);
        lvgl_port_unlock();
        return ESP_OK;
}

void main_screen_settings_button_cb(lv_event_t *e) {
        lvgl_port_lock(0);
        create_settings_screen(&settings_screen);
        lv_screen_load(settings_screen.scr);
        lvgl_port_unlock();
}
void create_main_screen(void) {
        lvgl_port_lock(0);
        main_screen.scr = lv_obj_create(NULL);

        lv_obj_set_style_bg_image_src(main_screen.scr, &winter, 0);
        lv_obj_remove_flag(main_screen.scr, LV_OBJ_FLAG_SCROLLABLE);

        create_settings_button(main_screen.scr, LV_ALIGN_TOP_RIGHT, -20, 10);
        create_wifi_status_label(main_screen.scr, LV_ALIGN_TOP_RIGHT, -70, 10);
        create_target_temperature_label(main_screen.scr, LV_ALIGN_BOTTOM_LEFT,
                                        100, -100);
        create_indoor_temperature_label(main_screen.scr, LV_ALIGN_BOTTOM_RIGHT,
                                        -170, -10);

        create_indoor_humidity_label(main_screen.scr, LV_ALIGN_BOTTOM_RIGHT,
                                     -50, -10);
        create_outdoor_temperature_label(main_screen.scr, LV_ALIGN_TOP_LEFT,
                                         150, 130);
        create_outdoor_humidity_label(main_screen.scr, LV_ALIGN_TOP_LEFT, 150,
                                      190);

        create_current_time_label(main_screen.scr, LV_ALIGN_RIGHT_MID, -100, 0);

        create_weather_icon_label(main_screen.scr, LV_ALIGN_TOP_LEFT, 100, 40);

        main_screen.target_temperature_slider =
            lv_slider_create(main_screen.scr);
        create_slider_with_gradient(main_screen.target_temperature_slider);
        lv_obj_remove_flag(main_screen.target_temperature_slider,
                           LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_add_event_cb(main_screen.target_temperature_slider,
                            slider_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_add_event_cb(main_screen.settings_button,
                            main_screen_settings_button_cb, LV_EVENT_PRESSED,
                            NULL);

        lvgl_port_unlock();
}

void create_wifi_status_label(lv_obj_t *scr, lv_align_t align, int16_t offset_x,
                              int16_t offset_y) {
        lvgl_port_lock(0);
        main_screen.wifi_status_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.wifi_status_shadow_label,
                         &material_icon_shadow_style, 0);

        main_screen.wifi_status_label = lv_label_create(scr);
        lv_obj_set_style_text_font(main_screen.wifi_status_label, &material_32,
                                   0);
        lv_label_bind_text(main_screen.wifi_status_label, &wifi_status_subject,
                           "%s");
        lv_label_bind_text(main_screen.wifi_status_shadow_label,
                           &wifi_status_subject, "%s");
        lv_obj_align(main_screen.wifi_status_label, align, offset_x, offset_y);
        lv_obj_align(main_screen.wifi_status_shadow_label, align, offset_x + 2,
                     offset_y + 2);
        lvgl_port_unlock();
}
void create_settings_button(lv_obj_t *scr, lv_align_t align, int16_t offset_x,
                            int16_t offset_y) {
        lvgl_port_lock(0);
        main_screen.settings_button = lv_button_create(scr);
        lv_obj_set_size(main_screen.settings_button, 40, 40);
        lv_obj_align(main_screen.settings_button, align, offset_x, offset_y);
        lv_obj_set_style_bg_opa(main_screen.settings_button, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(main_screen.settings_button, 0, 0);

        main_screen.settings_button_shadow_label =
            lv_label_create(main_screen.settings_button);
        lv_label_set_text_fmt(main_screen.settings_button_shadow_label, "%s",
                              LV_SYMBOL_SETTINGS);
        lv_obj_add_style(main_screen.settings_button_shadow_label,
                         &shadow_style_32, 0);
        lv_obj_align_to(main_screen.settings_button_shadow_label,
                        main_screen.settings_button, LV_ALIGN_CENTER, 2, 2);

        main_screen.settings_button_label =
            lv_label_create(main_screen.settings_button);
        lv_label_set_text_fmt(main_screen.settings_button_label, "%s",
                              LV_SYMBOL_SETTINGS);
        lv_obj_set_style_text_font(main_screen.settings_button_label,
                                   &lv_font_montserrat_32, 0);
        lv_obj_center(main_screen.settings_button_label);
        lv_obj_set_ext_click_area(main_screen.settings_button, 30);
        lvgl_port_unlock();
}
void create_target_temperature_label(lv_obj_t *scr, lv_align_t align,
                                     int16_t offset_x, int16_t offset_y) {
        lvgl_port_lock(0);
        main_screen.target_temperature_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.target_temperature_shadow_label,
                         &shadow_style_48, 0);
        lv_label_bind_text(main_screen.target_temperature_shadow_label,
                           &target_temperature_subject, "%d");

        main_screen.target_temperature_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.target_temperature_label,
                         &main_ui_style_48, 0);
        lv_label_bind_text(main_screen.target_temperature_label,
                           &target_temperature_subject, "%d");
        lv_label_set_text(main_screen.target_temperature_label, "70");
        lv_obj_align(main_screen.target_temperature_label, align, offset_x,
                     offset_y);
        lv_obj_align(main_screen.target_temperature_shadow_label, align,
                     offset_x + 2, offset_y + 2);
        lvgl_port_unlock();
}
void create_indoor_temperature_label(lv_obj_t *scr, lv_align_t align,
                                     int16_t offset_x, int16_t offset_y) {
        lvgl_port_lock(0);
        main_screen.indoor_temperature_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.indoor_temperature_shadow_label,
                         &shadow_style_40, 0);
        lv_label_bind_text(main_screen.indoor_temperature_shadow_label,
                           &indoor_temperature_subject, "%d");

        main_screen.indoor_temperature_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.indoor_temperature_label,
                         &main_ui_style_40, 0);
        lv_label_bind_text(main_screen.indoor_temperature_label,
                           &indoor_temperature_subject, "%d");

        main_screen.indoor_temperature_icon_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.indoor_temperature_icon_shadow_label,
                         &weather_icon_shadow_style_40, 0);
        lv_label_set_text(main_screen.indoor_temperature_icon_shadow_label,
                          WI_THERMOMETER);

        main_screen.indoor_temperature_icon_label = lv_label_create(scr);
        lv_obj_set_style_text_font(main_screen.indoor_temperature_icon_label,
                                   &weather_32, 0);
        lv_label_set_text(main_screen.indoor_temperature_icon_label,
                          WI_THERMOMETER);

        lv_obj_align(main_screen.indoor_temperature_label, align, offset_x,
                     offset_y);
        lv_obj_align(main_screen.indoor_temperature_shadow_label, align,
                     offset_x + 2, offset_y + 2);
        lv_obj_align(main_screen.indoor_temperature_icon_label, align,
                     offset_x + 30, offset_y);
        lv_obj_align(main_screen.indoor_temperature_icon_shadow_label, align,
                     offset_x + 32, offset_y + 2);
        lvgl_port_unlock();
}
void create_indoor_humidity_label(lv_obj_t *scr, lv_align_t align,
                                  int16_t offset_x, int16_t offset_y) {
        lvgl_port_lock(0);
        main_screen.indoor_humidity_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.indoor_humidity_shadow_label,
                         &shadow_style_40, 0);
        lv_label_bind_text(main_screen.indoor_humidity_shadow_label,
                           &indoor_humidity_subject, "%d");

        main_screen.indoor_humidity_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.indoor_humidity_label, &main_ui_style_40,
                         0);
        lv_label_bind_text(main_screen.indoor_humidity_label,
                           &indoor_humidity_subject, "%d");

        main_screen.indoor_humidity_icon_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.indoor_humidity_icon_shadow_label,
                         &weather_icon_shadow_style_32, 0);
        lv_label_set_text(main_screen.indoor_humidity_icon_shadow_label,
                          WI_HUMIDITY);

        main_screen.indoor_humidity_icon_label = lv_label_create(scr);
        lv_obj_set_style_text_font(main_screen.indoor_humidity_icon_label,
                                   &weather_32, 0);
        lv_label_set_text(main_screen.indoor_humidity_icon_label, WI_HUMIDITY);

        lv_obj_align(main_screen.indoor_humidity_label, align, offset_x,
                     offset_y);
        lv_obj_align(main_screen.indoor_humidity_shadow_label, align,
                     offset_x + 2, offset_y + 2);
        lv_obj_align(main_screen.indoor_humidity_icon_label, align,
                     offset_x + 30, offset_y);
        lv_obj_align(main_screen.indoor_humidity_icon_shadow_label, align,
                     offset_x + 32, offset_y + 2);
        lvgl_port_unlock();
}
void create_outdoor_temperature_label(lv_obj_t *scr, lv_align_t align,
                                      int16_t offset_x, int16_t offset_y) {
        lvgl_port_lock(0);
        main_screen.outdoor_temperature_icon_shadow_label =
            lv_label_create(scr);
        lv_obj_add_style(main_screen.outdoor_temperature_icon_shadow_label,
                         &weather_icon_shadow_style_32, 0);
        lv_label_set_text(main_screen.outdoor_temperature_icon_shadow_label,
                          WI_THERMOMETER);

        main_screen.outdoor_temperature_icon_label = lv_label_create(scr);
        lv_obj_set_style_text_font(main_screen.outdoor_temperature_icon_label,
                                   &weather_32, 0);
        lv_obj_set_style_text_color(main_screen.outdoor_temperature_icon_label,
                                    lv_color_white(), 0);
        lv_label_set_text(main_screen.outdoor_temperature_icon_label,
                          WI_THERMOMETER);

        main_screen.outdoor_temperature_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.outdoor_temperature_shadow_label,
                         &shadow_style_40, 0);
        lv_label_bind_text(main_screen.outdoor_temperature_shadow_label,
                           &outdoor_temperature_subject, "%d");

        main_screen.outdoor_temperature_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.outdoor_temperature_label,
                         &main_ui_style_40, 0);
        lv_label_bind_text(main_screen.outdoor_temperature_label,
                           &outdoor_temperature_subject, "%d");
        lv_obj_align(main_screen.outdoor_temperature_label, align, offset_x,
                     offset_y);
        lv_obj_align(main_screen.outdoor_temperature_shadow_label, align,
                     offset_x + 2, offset_y + 2);
        lv_obj_align(main_screen.outdoor_temperature_icon_label, align,
                     offset_x - 45, offset_y);
        lv_obj_align(main_screen.outdoor_temperature_icon_shadow_label, align,
                     offset_x - 43, offset_y + 2);
        lvgl_port_unlock();
}
void create_outdoor_humidity_label(lv_obj_t *scr, lv_align_t align,
                                   int16_t offset_x, int16_t offset_y) {
        lvgl_port_lock(0);
        main_screen.outdoor_humidity_icon_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.outdoor_humidity_icon_shadow_label,
                         &weather_icon_shadow_style_32, 0);
        lv_label_set_text(main_screen.outdoor_humidity_icon_shadow_label,
                          WI_HUMIDITY);

        main_screen.outdoor_humidity_icon_label = lv_label_create(scr);
        lv_obj_set_style_text_font(main_screen.outdoor_humidity_icon_label,
                                   &weather_32, 0);
        lv_obj_set_style_text_color(main_screen.outdoor_humidity_icon_label,
                                    lv_color_white(), 0);
        lv_label_set_text(main_screen.outdoor_humidity_icon_label, WI_HUMIDITY);

        main_screen.outdoor_humidity_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.outdoor_humidity_shadow_label,
                         &shadow_style_40, 0);
        lv_label_bind_text(main_screen.outdoor_humidity_shadow_label,
                           &outdoor_humidity_subject, "%d");

        main_screen.outdoor_humidity_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.outdoor_humidity_label, &main_ui_style_40,
                         0);
        lv_label_bind_text(main_screen.outdoor_humidity_label,
                           &outdoor_humidity_subject, "%d");
        lv_obj_align(main_screen.outdoor_humidity_label, align, offset_x,
                     offset_y);
        lv_obj_align(main_screen.outdoor_humidity_shadow_label, align,
                     offset_x + 2, offset_y + 2);
        lv_obj_align(main_screen.outdoor_humidity_icon_label, align,
                     offset_x - 45, offset_y);
        lv_obj_align(main_screen.outdoor_humidity_icon_shadow_label, align,
                     offset_x - 43, offset_y + 2);
        lvgl_port_unlock();
}
void create_weather_icon_label(lv_obj_t *scr, lv_align_t align,
                               int16_t offset_x, int16_t offset_y) {
        lvgl_port_lock(0);
        main_screen.weather_icon_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.weather_icon_shadow_label,
                         &weather_icon_shadow_style_60, 0);
        lv_label_bind_text(main_screen.weather_icon_shadow_label,
                           &weather_icon_subject, "%s");

        main_screen.weather_icon_label = lv_label_create(scr);
        lv_obj_set_style_text_font(main_screen.weather_icon_label, &weather_60,
                                   0);
        lv_label_bind_text(main_screen.weather_icon_label,
                           &weather_icon_subject, "%s");

        lv_grad_dsc_t grad;
        grad.dir = LV_GRAD_DIR_HOR;
        grad.stops_count = 2;
        grad.stops[0].color = lv_color_black();
        grad.stops[0].opa = LV_OPA_COVER;
        grad.stops[0].frac = 0;
        grad.stops[1].color = lv_color_black();
        grad.stops[1].opa = LV_OPA_TRANSP;
        grad.stops[1].frac = 255;

        main_screen.weather_conditions_container = lv_obj_create(scr);
        lv_obj_set_size(main_screen.weather_conditions_container, 300, 40);
        lv_obj_set_style_border_width(main_screen.weather_conditions_container,
                                      0, 0);
        lv_obj_set_style_bg_opa(main_screen.weather_conditions_container,
                                LV_OPA_TRANSP, 0);
        lv_obj_set_style_bg_grad_dir(main_screen.weather_conditions_container,
                                     LV_GRAD_DIR_HOR, 0);
        lv_obj_set_style_bg_grad_color(main_screen.weather_conditions_container,
                                       lv_color_hex(0xffffff), 0);
        lv_obj_set_style_bg_main_opa(main_screen.weather_conditions_container,
                                     LV_OPA_TRANSP, 0);
        lv_obj_set_style_bg_grad_opa(main_screen.weather_conditions_container,
                                     LV_OPA_COVER, 0);
        lv_obj_set_style_clip_corner(main_screen.weather_conditions_container,
                                     true, 0);
        lv_obj_set_style_bg_grad(main_screen.weather_conditions_container,
                                 &grad, 0);

        main_screen.weather_conditions_shadow_label =
            lv_label_create(main_screen.weather_conditions_container);
        lv_obj_add_style(main_screen.weather_conditions_shadow_label,
                         &shadow_style_24, 0);
        lv_obj_set_width(main_screen.weather_conditions_shadow_label,
                         lv_pct(100));
        lv_label_set_long_mode(main_screen.weather_conditions_shadow_label,
                               LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_label_bind_text(main_screen.weather_conditions_shadow_label,
                           &weather_conditions_subject, "%s");

        main_screen.weather_conditions_label =
            lv_label_create(main_screen.weather_conditions_container);
        lv_obj_add_style(main_screen.weather_conditions_label,
                         &main_ui_style_24, 0);
        lv_obj_set_width(main_screen.weather_conditions_label, lv_pct(100));
        lv_label_set_long_mode(main_screen.weather_conditions_label,
                               LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_label_bind_text(main_screen.weather_conditions_label,
                           &weather_conditions_subject, "%s");

        lv_obj_set_scrollbar_mode(main_screen.weather_conditions_container,
                                  LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_scrollbar_mode(main_screen.weather_conditions_shadow_label,
                                  LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_scrollbar_mode(main_screen.weather_conditions_label,
                                  LV_SCROLLBAR_MODE_OFF);

        lv_obj_align(main_screen.weather_icon_label, align, offset_x, offset_y);
        lv_obj_align(main_screen.weather_icon_shadow_label, align, offset_x + 2,
                     offset_y + 2);
        lv_obj_align(main_screen.weather_conditions_container, align,
                     offset_x + 100, offset_y);
        lv_obj_align(main_screen.weather_conditions_shadow_label,
                     LV_ALIGN_CENTER, 2, 2);
        lv_obj_align(main_screen.weather_conditions_label, LV_ALIGN_CENTER, 0,
                     0);
        lvgl_port_unlock();
}
void create_current_time_label(lv_obj_t *scr, lv_align_t align,
                               int16_t offset_x, int16_t offset_y) {
        lvgl_port_lock(0);
        main_screen.time_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.time_shadow_label, &shadow_style_48, 0);
        lv_label_bind_text(main_screen.time_shadow_label, &time_subject, "%s");

        main_screen.date_shadow_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.date_shadow_label, &shadow_style_40, 0);
        lv_label_bind_text(main_screen.date_shadow_label, &date_subject, "%s");

        main_screen.time_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.time_label, &main_ui_style_48, 0);
        lv_label_bind_text(main_screen.time_label, &time_subject, "%s");

        main_screen.date_label = lv_label_create(scr);
        lv_obj_add_style(main_screen.date_label, &main_ui_style_40, 0);
        lv_label_bind_text(main_screen.date_label, &date_subject, "%s");

        lv_obj_align(main_screen.time_label, align, offset_x, offset_y);
        lv_obj_align(main_screen.time_shadow_label, align, offset_x + 2,
                     offset_y + 2);
        lv_obj_align(main_screen.date_label, align, offset_x, offset_y + 50);
        lv_obj_align(main_screen.date_shadow_label, align, offset_x + 2,
                     offset_y + 42);
        lvgl_port_unlock();
}
