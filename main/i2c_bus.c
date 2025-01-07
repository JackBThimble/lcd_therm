#include "i2c_bus.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "soc/clk_tree_defs.h"

i2c_master_bus_handle_t i2c_bus_handle;

esp_err_t
i2c_bus_init (void)
{
        i2c_master_bus_config_t bus_cfg
            = { .scl_io_num = I2C_MASTER_SCL_IO,
                .sda_io_num = I2C_MASTER_SDA_IO,
                .clk_source = I2C_CLK_SRC_DEFAULT,
                .glitch_ignore_cnt = 7,
                .i2c_port = I2C_MASTER_NUM,
                .flags.allow_pd = 0,
                .flags.enable_internal_pullup = 0 };

        return i2c_new_master_bus (&bus_cfg, &i2c_bus_handle);
}
