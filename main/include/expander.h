#pragma once
#include "esp_err.h"

#define CH422G_ADDR 0x20
#define CH422G_REG_MODE 0x24
#define CH422G_REG_IN 0x26
#define CH422G_REG_OUT 0x38
#define CH422G_REG_OUT_UPPER 0x23

#define CH422G_MODE_OUTPUT 0x01
#define CH422G_MODE_OPEN_DRAIN 0x04

esp_err_t expander_init ();
esp_err_t expander_set_level (uint8_t pin, uint8_t level);
esp_err_t expander_get_level (uint8_t pin, uint8_t *level);
