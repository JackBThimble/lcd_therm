#pragma once

#include "esp_err.h"
#include "esp_lvgl_port.h"

#define BUTTON_DEBOUNCE_MS 300

esp_err_t lcd_init(void);
esp_err_t touch_init(void);
esp_err_t lvgl_init(void);
