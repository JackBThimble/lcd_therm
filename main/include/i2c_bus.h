#pragma once

#include "driver/i2c_master.h"

extern i2c_master_bus_handle_t i2c_bus_handle;
#define I2C_MASTER_SCL_IO 9
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_NUM 0
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TIMEOUT_MS 1000

esp_err_t i2c_bus_init (void);
