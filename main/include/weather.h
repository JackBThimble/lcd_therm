#pragma once

#include "esp_err.h"

typedef struct
{
        float temperature;
        float humidity;
        char condition[32];
} weather_data_t;
esp_err_t weather_init (void);
esp_err_t weather_get_current (weather_data_t *weather);
