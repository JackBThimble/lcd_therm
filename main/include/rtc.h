#pragma once

#include "esp_err.h"
#include <time.h>

esp_err_t ext_rtc_init(void);
esp_err_t rtc_set_time(const struct tm *time);
esp_err_t rtc_get_time(struct tm *time);
