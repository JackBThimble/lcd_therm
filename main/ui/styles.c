#include "styles.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "fonts/lcd_therm_fonts.h"

static const char *TAG = "UI Styles";

// Styles
lv_style_t shadow_style_24;
lv_style_t shadow_style_48;
lv_style_t weather_icon_shadow_style_32;
lv_style_t weather_icon_shadow_style_40;
lv_style_t weather_icon_shadow_style_48;
lv_style_t weather_icon_shadow_style_60;
lv_style_t weather_icon_style_32;
lv_style_t weather_icon_style_40;
lv_style_t weather_icon_style_48;
lv_style_t material_icon_shadow_style;
lv_style_t shadow_style_40;
lv_style_t shadow_style_32;
lv_style_t main_ui_style_24;
lv_style_t main_ui_style_48;
lv_style_t main_ui_style_40;
lv_style_t main_ui_style_32;

void init_styles(void) {
        lvgl_port_lock(0);
        ESP_LOGI(TAG, "Initializing styles");

        lv_style_init(&shadow_style_24);
        lv_style_set_text_color(&shadow_style_24, lv_color_black());
        lv_style_set_text_font(&shadow_style_24, &lv_font_montserrat_24);
        lv_style_set_text_opa(&shadow_style_24, LV_OPA_70);

        lv_style_init(&shadow_style_32);
        lv_style_set_text_color(&shadow_style_32, lv_color_black());
        lv_style_set_text_font(&shadow_style_32, &lv_font_montserrat_32);
        lv_style_set_text_opa(&shadow_style_32, LV_OPA_70);

        lv_style_init(&shadow_style_40);
        lv_style_set_text_color(&shadow_style_40, lv_color_black());
        lv_style_set_text_font(&shadow_style_40, &lv_font_montserrat_40);
        lv_style_set_text_opa(&shadow_style_40, LV_OPA_70);

        lv_style_init(&shadow_style_48);
        lv_style_set_text_color(&shadow_style_48, lv_color_black());
        lv_style_set_text_font(&shadow_style_48, &lv_font_montserrat_48);
        lv_style_set_text_opa(&shadow_style_48, LV_OPA_70);

        lv_style_init(&weather_icon_shadow_style_32);
        lv_style_set_text_color(&weather_icon_shadow_style_32,
                                lv_color_black());
        lv_style_set_text_font(&weather_icon_shadow_style_32, &weather_32);
        lv_style_set_text_opa(&weather_icon_shadow_style_32, LV_OPA_70);

        lv_style_init(&weather_icon_shadow_style_40);
        lv_style_set_text_color(&weather_icon_shadow_style_40,
                                lv_color_black());
        lv_style_set_text_font(&weather_icon_shadow_style_40, &weather_40);
        lv_style_set_text_opa(&weather_icon_shadow_style_40, LV_OPA_70);

        lv_style_init(&weather_icon_shadow_style_48);
        lv_style_set_text_color(&weather_icon_shadow_style_48,
                                lv_color_black());
        lv_style_set_text_font(&weather_icon_shadow_style_48, &weather_48);
        lv_style_set_text_opa(&weather_icon_shadow_style_48, LV_OPA_70);

        lv_style_init(&weather_icon_shadow_style_60);
        lv_style_set_text_color(&weather_icon_shadow_style_60,
                                lv_color_black());
        lv_style_set_text_font(&weather_icon_shadow_style_60, &weather_60);
        lv_style_set_text_opa(&weather_icon_shadow_style_60, LV_OPA_70);

        lv_style_init(&material_icon_shadow_style);
        lv_style_set_text_color(&material_icon_shadow_style, lv_color_black());
        lv_style_set_text_font(&material_icon_shadow_style, &material_32);
        lv_style_set_text_opa(&material_icon_shadow_style, LV_OPA_70);

        lv_style_init(&main_ui_style_24);
        lv_style_set_text_color(&main_ui_style_24, lv_color_white());
        lv_style_set_text_font(&main_ui_style_24, &lv_font_montserrat_24);
        lv_style_set_text_opa(&main_ui_style_24, LV_OPA_100);

        lv_style_init(&main_ui_style_32);
        lv_style_set_text_color(&main_ui_style_32, lv_color_white());
        lv_style_set_text_font(&main_ui_style_32, &lv_font_montserrat_32);
        lv_style_set_text_opa(&main_ui_style_32, LV_OPA_100);

        lv_style_init(&main_ui_style_40);
        lv_style_set_text_color(&main_ui_style_40, lv_color_white());
        lv_style_set_text_font(&main_ui_style_40, &lv_font_montserrat_40);
        lv_style_set_text_opa(&main_ui_style_40, LV_OPA_100);

        lv_style_init(&main_ui_style_48);
        lv_style_set_text_color(&main_ui_style_48, lv_color_white());
        lv_style_set_text_font(&main_ui_style_48, &lv_font_montserrat_48);
        lv_style_set_text_opa(&main_ui_style_48, LV_OPA_100);
        lvgl_port_unlock();
}
