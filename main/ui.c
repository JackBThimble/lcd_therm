#include "ui.h"
#include "app_settings.h"
#include "core/lv_obj.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "fonts/material_32.h"
#include "fonts/weather_32.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"
#include "images/frans.h"
#include "images/liquid.h"
#include "images/thekid.h"
#include "images/winter.h"
#include "lv_port.h"
#include "lvgl.h"
#include "misc/lv_anim.h"
#include "weather_icons.h"
#include "wifi_icons.h"

void create_slider_with_gradient(lv_obj_t *slider);
void init_styles(void);
void init_subjects(void);

extern int32_t target_temperature;
extern int32_t indoor_temperature;
extern int32_t outdoor_temperature;
extern int32_t indoor_humidity;
extern int32_t outdoor_humidity;
extern char weather_conditions_icon[8];
extern char wifi_status_icon[8];
extern char current_time_string[16];
extern char current_date_string[64];

static const char *TAG = "UI";
// labels
lv_obj_t *target_temperature_label;
lv_obj_t *indoor_temperature_label;
lv_obj_t *outdoor_temperature_label;
lv_obj_t *target_temperature_shadow_label;
lv_obj_t *indoor_temperature_shadow_label;
lv_obj_t *outdoor_temperature_shadow_label;
lv_obj_t *wifi_status_label;
lv_obj_t *indoor_humidity_label;
lv_obj_t *outdoor_humidity_label;
lv_obj_t *indoor_humidity_shadow_label;
lv_obj_t *outdoor_humidity_shadow_label;
lv_obj_t *time_label;
lv_obj_t *time_shadow_label;
lv_obj_t *date_label;
lv_obj_t *date_shadow_label;

// buttons
lv_obj_t *settings_button;
lv_obj_t *target_temperature_slider;

// styles
lv_style_t shadow_style_48;
lv_style_t weather_icon_shadow_style;
lv_style_t shadow_style_40;
lv_style_t shadow_style_32;
lv_style_t main_ui_style_48;
lv_style_t main_ui_style_32;

// screens
lv_obj_t *main_screen;

// subjects
lv_subject_t target_temperature_subject;
lv_subject_t wifi_status_subject;
lv_subject_t indoor_temperature_subject;
lv_subject_t outdoor_temperature_subject;
lv_subject_t indoor_humidity_subject;
lv_subject_t outdoor_humidity_subject;
lv_subject_t time_subject;
lv_subject_t date_subject;
lv_subject_t weather_icon_subject;

void slider_event_handler(lv_event_t *e) {
        lvgl_port_lock(-1);
        lv_obj_t *slider = lv_event_get_target(e);
        int value = lv_slider_get_value(slider);

        int normalized_value =
            ((value - TEMP_MIN) * 255) / (TEMP_MAX - TEMP_MIN);
        lv_color_t color =
            lv_color_mix(lv_color_hex(0x0000ff), lv_color_hex(0xff0000),
                         255 - normalized_value);

        lv_obj_set_style_bg_color(slider, color, LV_PART_KNOB);

        lvgl_port_unlock();
}

void set_background_image(lv_obj_t *parent, const lv_image_dsc_t *src) {
        ESP_LOGI(TAG, "Setting background image");
        lv_obj_t *bg_img = lv_image_create(parent);

        lv_image_set_src(bg_img, src);
        lv_obj_set_size(bg_img, lv_obj_get_width(parent),
                        lv_obj_get_height(parent));
        lv_obj_center(bg_img);
}

esp_err_t create_ui(void) {
        /* init_subjects(); */
        init_styles();
        main_screen = lv_obj_create(NULL);

        set_background_image(main_screen, &liquid);
        lv_obj_remove_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);

        /* settings_button = lv_button_create(main_screen); */
        /* lv_obj_set_size(settings_button, 40, 40); */
        /* lv_obj_align(settings_button, LV_ALIGN_TOP_RIGHT, -20, 10); */
        /* lv_obj_set_style_bg_opa(settings_button, LV_OPA_TRANSP, 0); */
        /* lv_obj_set_style_border_width(settings_button, 0, 0); */
        /**/
        /* lv_obj_t *settings_label = lv_label_create(settings_button); */
        /* lv_label_set_text_fmt(settings_label, "%s", LV_SYMBOL_SETTINGS); */
        /* lv_obj_set_style_text_font(settings_label, &lv_font_montserrat_32,
         * 0); */
        /* lv_obj_center(settings_label); */
        /**/
        /* wifi_status_label = lv_label_create(main_screen); */
        /* lv_obj_set_style_text_font(wifi_status_label, &material_32, 0); */
        /* lv_label_bind_text(wifi_status_label, &wifi_status_subject, "%s");
         */
        /* lv_label_set_text(wifi_status_label, WIFI_ICON_1_BARS); */
        /* lv_obj_align(wifi_status_label, LV_ALIGN_TOP_RIGHT, -70, 10); */
        /**/
        /* indoor_temperature_label = lv_label_create(main_screen); */
        /* lv_obj_add_style(indoor_temperature_label, &main_ui_style_32, 0);
         */
        /* lv_label_bind_text(indoor_temperature_label, */
        /*                    &indoor_temperature_subject, "%d"); */
        /* lv_label_set_text(indoor_temperature_label, "72"); */
        /* lv_obj_align(indoor_temperature_label, LV_ALIGN_TOP_MID, 0, 10);
         */
        /**/
        /* indoor_humidity_label = lv_label_create(main_screen); */
        /* lv_obj_add_style(indoor_humidity_label, &main_ui_style_32, 0); */
        /* lv_label_bind_text(indoor_humidity_label,
           &indoor_humidity_subject, */
        /* "%d"); */
        /* lv_label_set_text(indoor_humidity_label, "32"); */
        /* lv_obj_align(indoor_humidity_label, LV_ALIGN_TOP_MID, 0, 50); */
        /**/
        /* outdoor_temperature_label = lv_label_create(main_screen); */
        /* lv_obj_add_style(outdoor_temperature_label, &main_ui_style_32,
           0); */
        /* lv_label_bind_text(outdoor_temperature_label, */
        /* &outdoor_temperature_subject, "%d"); */
        /* lv_label_set_text(outdoor_temperature_label, "29"); */
        /* lv_obj_align(outdoor_temperature_label, LV_ALIGN_TOP_LEFT, 50,
           120); */
        /**/
        /* outdoor_humidity_label = lv_label_create(main_screen); */
        /* lv_obj_add_style(outdoor_humidity_label, &main_ui_style_32, 0);
         */
        /* lv_label_bind_text(outdoor_humidity_label,
           &outdoor_humidity_subject, */
        /*                    "%d"); */
        /* lv_label_set_text(outdoor_humidity_label, "55"); */
        /* lv_obj_align(outdoor_humidity_label, LV_ALIGN_TOP_LEFT, 50, 170);
         */
        /**/
        /* time_label = lv_label_create(main_screen); */
        /* lv_obj_add_style(time_label, &main_ui_style_48, 0); */
        /* lv_label_bind_text(time_label, &time_subject, "%s"); */
        /* lv_label_set_text(time_label, "12:00 PM"); */
        /* lv_obj_align(time_label, LV_ALIGN_RIGHT_MID, -100, 0); */
        /**/
        target_temperature_label = lv_label_create(main_screen);
        lv_obj_add_style(target_temperature_label, &main_ui_style_48, 0);
        lv_label_bind_text(target_temperature_label,
                           &target_temperature_subject, "%d");
        lv_label_set_text(target_temperature_label, "70");
        lv_obj_align(target_temperature_label, LV_ALIGN_BOTTOM_LEFT, 100, -100);
        target_temperature_slider = lv_slider_create(main_screen);
        create_slider_with_gradient(target_temperature_slider);
        lv_obj_remove_flag(target_temperature_slider, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(target_temperature_slider, slider_event_handler,
                            LV_EVENT_VALUE_CHANGED, NULL);

        lv_screen_load(main_screen);

        return ESP_OK;
}

void create_slider_with_gradient(lv_obj_t *slider) {
        ESP_LOGI(TAG, "Creating slider with gradient");
        lv_slider_set_range(slider, TEMP_MIN, TEMP_MAX);
        lv_slider_set_mode(slider, LV_SLIDER_MODE_NORMAL);
        lv_obj_set_size(slider, 10, 150);
        lv_obj_align(slider, LV_ALIGN_BOTTOM_LEFT, 60, -50);

        static lv_style_t style_indicator;
        lv_style_init(&style_indicator);
        lv_style_set_bg_grad_color(&style_indicator, lv_color_hex(0x0000ff));
        lv_style_set_bg_color(&style_indicator, lv_color_hex(0xff0000));
        lv_style_set_bg_grad_dir(&style_indicator, LV_GRAD_DIR_VER);
        lv_style_set_width(&style_indicator, 1);

        int32_t normalized_value =
            ((target_temperature - TEMP_MIN) * 255) / (TEMP_MAX - TEMP_MIN);
        lv_color_t color =
            lv_color_mix(lv_color_hex(0x0000ff), lv_color_hex(0xff0000),
                         255 - normalized_value);

        static lv_style_t style_knob;
        lv_style_init(&style_knob);
        lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
        lv_style_set_size(&style_knob, 10, 10);
        lv_style_set_border_width(&style_knob, 0);
        lv_style_set_border_color(&style_knob, lv_color_black());
        lv_style_set_bg_color(&style_knob, color);

        /* lv_slider_bind_value(slider, &target_temperature_subject); */
        lv_slider_set_value(slider, 70, LV_ANIM_OFF);

        lv_obj_add_style(slider, &style_knob, LV_PART_KNOB);
        lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);
}

void init_styles(void) {
        lvgl_port_lock(-1);
        ESP_LOGI(TAG, "Initializing styles");
        lv_style_init(&shadow_style_32);
        lv_style_set_text_color(&shadow_style_32, lv_color_black());
        lv_style_set_text_font(&shadow_style_32, &lv_font_montserrat_32);
        lv_style_set_text_opa(&shadow_style_32, LV_OPA_70);

        lv_style_init(&shadow_style_40);
        lv_style_set_text_color(&shadow_style_32, lv_color_black());
        lv_style_set_text_font(&shadow_style_32, &lv_font_montserrat_40);
        lv_style_set_text_opa(&shadow_style_32, LV_OPA_70);

        lv_style_init(&shadow_style_48);
        lv_style_set_text_color(&shadow_style_48, lv_color_black());
        lv_style_set_text_font(&shadow_style_48, &lv_font_montserrat_48);
        lv_style_set_text_opa(&shadow_style_48, LV_OPA_70);

        lv_style_init(&weather_icon_shadow_style);
        lv_style_set_text_color(&weather_icon_shadow_style, lv_color_black());
        lv_style_set_text_font(&weather_icon_shadow_style, &weather_32);
        lv_style_set_text_opa(&weather_icon_shadow_style, LV_OPA_70);

        lv_style_init(&main_ui_style_32);
        lv_style_set_text_color(&main_ui_style_32, lv_color_white());
        lv_style_set_text_font(&main_ui_style_32, &lv_font_montserrat_32);
        lv_style_set_text_opa(&main_ui_style_32, LV_OPA_100);

        lv_style_init(&main_ui_style_48);
        lv_style_set_text_color(&main_ui_style_48, lv_color_white());
        lv_style_set_text_font(&main_ui_style_48, &lv_font_montserrat_48);
        lv_style_set_text_opa(&main_ui_style_48, LV_OPA_100);

        lvgl_port_unlock();
}

void init_subjects(void) {
        lvgl_port_lock(-1);
        ESP_LOGI(TAG, "Initializing subjects");
        lv_subject_init_int(&target_temperature_subject, target_temperature);
        lv_subject_init_int(&indoor_temperature_subject, indoor_temperature);
        lv_subject_init_int(&outdoor_temperature_subject, outdoor_temperature);
        lv_subject_init_int(&indoor_humidity_subject, indoor_humidity);
        lv_subject_init_int(&outdoor_humidity_subject, outdoor_humidity);
        lv_subject_init_string(&weather_icon_subject, weather_conditions_icon,
                               NULL, 8, weather_conditions_icon);
        lv_subject_init_string(&wifi_status_subject, wifi_status_icon, NULL, 8,
                               wifi_status_icon);
        lv_subject_init_string(&time_subject, current_time_string, NULL, 16,
                               current_time_string);
        lv_subject_init_string(&date_subject, current_date_string, NULL, 64,
                               current_date_string);
        lvgl_port_unlock();
}
