#pragma once

#include "esp_err.h"

esp_err_t temp_sensor_init (void);
esp_err_t temp_sensor_read (float *temperature, float *humidity);
