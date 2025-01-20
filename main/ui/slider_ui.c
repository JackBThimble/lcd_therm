#include "slider_ui.h"
#include "app_settings.h"
#include "lv_port.h"
#include "main.h"
#include "subjects_ui.h"
#include "ui.h"

void slider_event_handler(lv_event_t *e) {
        lvgl_port_lock(0);
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

void create_slider_with_gradient(lv_obj_t *slider) {
        lvgl_port_lock(0);
        lv_slider_set_range(slider, TEMP_MIN, TEMP_MAX);
        lv_slider_set_mode(slider, LV_SLIDER_MODE_NORMAL);
        lv_obj_set_size(slider, 10, 160);
        lv_obj_align(slider, LV_ALIGN_BOTTOM_LEFT, 60, -50);

        static lv_style_t style_indicator;
        lv_style_init(&style_indicator);
        lv_style_set_bg_color(&style_indicator, lv_color_hex(0xff0000));
        lv_style_set_bg_grad_color(&style_indicator, lv_color_hex(0x0000ff));
        lv_style_set_bg_grad_dir(&style_indicator, LV_GRAD_DIR_VER);
        /* lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER); */
        lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);

        int32_t normalized_value =
            ((g_target_temperature - TEMP_MIN) * 255) / (TEMP_MAX - TEMP_MIN);
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

        lv_slider_bind_value(slider, &target_temperature_subject);

        lv_obj_set_ext_click_area(slider, 50);
        lv_obj_add_style(slider, &style_knob, LV_PART_KNOB);
        lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);
        lvgl_port_unlock();
}
