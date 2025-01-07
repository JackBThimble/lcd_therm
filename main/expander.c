#include "expander.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "i2c_bus.h"
#include <unistd.h>

static const char *TAG = "CH422G";
static i2c_master_dev_handle_t expander_mode_handle;
static i2c_master_dev_handle_t expander_out_handle;
static i2c_master_dev_handle_t expander_out_upper_handle;
static i2c_master_dev_handle_t expander_in_handle;
static uint8_t output_state_low = 0x2E;
static uint8_t output_state_high = 0;

static uint16_t output_bits = 0;

esp_err_t
expander_init (void)
{
        i2c_device_config_t mode_cfg = {
                .device_address = 0x24,
                .scl_speed_hz = 400000,
        };

        /* ESP_LOGI (TAG, "Adding CH422G mode address to the I2C bus"); */
        esp_err_t err = i2c_master_bus_add_device (i2c_bus_handle, &mode_cfg,
                                                   &expander_mode_handle);
        if (err != ESP_OK)
                {
                        ESP_LOGE (
                            TAG,
                            "Failed to add the CH422G mode address to the I2C "
                            "bus: %s",
                            esp_err_to_name (err));
                        return err;
                }

        i2c_device_config_t out_cfg = {
                .device_address = 0x38,
                .scl_speed_hz = 400000,
        };
        /* ESP_LOGI (TAG, "Adding CH422 ouput address to the i2c bus"); */
        err = i2c_master_bus_add_device (i2c_bus_handle, &out_cfg,
                                         &expander_out_handle);
        if (err != ESP_OK)
                {
                        ESP_LOGE (TAG,
                                  "Failed to add CH422 output address to I2C "
                                  "bus: %s",
                                  esp_err_to_name (err));
                        return err;
                }

        i2c_device_config_t out_upper_cfg = {
                .device_address = 0x23,
                .scl_speed_hz = 400000,
        };
        /* ESP_LOGI (TAG, "Adding CH422 upper output address to the i2c bus");
         */
        err = i2c_master_bus_add_device (i2c_bus_handle, &out_upper_cfg,
                                         &expander_out_upper_handle);
        if (err != ESP_OK)
                {
                        ESP_LOGE (TAG,
                                  "Failed to add CH422 upper output address "
                                  "to the I2C bus: %s",
                                  esp_err_to_name (err));
                        return err;
                }

        i2c_device_config_t in_cfg = {
                .device_address = 0x26,
                .scl_speed_hz = 400000,
        };
        err = i2c_master_bus_add_device (i2c_bus_handle, &in_cfg,
                                         &expander_in_handle);
        if (err != ESP_OK)
                {
                        ESP_LOGE (TAG,
                                  "Failed to add CH422 input address to the "
                                  "I2C bus: %s",
                                  esp_err_to_name (err));
                        return err;
                }

        uint8_t mode = CH422G_MODE_OUTPUT;
        /* ESP_LOGI (TAG, "Setting CH422G to output mode"); */
        err = i2c_master_transmit (expander_mode_handle, &mode, 1, -1);
        if (err != ESP_OK)
                {
                        ESP_LOGE (TAG,
                                  "Failed to set output mode on CH422G: %s",
                                  esp_err_to_name (err));
                }

        uint8_t write_buf = 0x2e;
        err = i2c_master_transmit (expander_out_handle, &write_buf, 1, -1);
        /* ESP_LOGI (TAG, "CH422G initialized successfully"); */
        return ESP_OK;
}

esp_err_t
expander_set_level (uint8_t pin, uint8_t level)
{
        // Set ch422g to output mode
        uint8_t write_buf = 0x01;
        i2c_master_transmit (expander_mode_handle, &write_buf, 1, -1);

        if (pin < 8)
                {
                        if (level)
                                {
                                        output_state_low |= (1 << pin);
                                }
                        else
                                {
                                        output_state_low &= ~(1 << pin);
                                }
                        return i2c_master_transmit (expander_out_handle,
                                                    &output_state_low, 1, -1);
                }
        else
                {
                        uint8_t adjusted_pin = pin - 8;
                        if (level)
                                {
                                        output_state_high
                                            |= (1 << adjusted_pin);
                                }
                        else
                                {
                                        output_state_high
                                            &= ~(1 << adjusted_pin);
                                }
                        return i2c_master_transmit (expander_out_upper_handle,
                                                    &output_state_high, 1, -1);
                }
}

esp_err_t
expander_get_level (uint8_t pin, uint8_t *level)
{
        uint8_t value;
        esp_err_t ret = i2c_master_receive (expander_in_handle, &value, 1, -1);

        if (ret == ESP_OK)
                {
                        *level = (value >> pin) & 0x01;
                        /* ESP_LOGI (TAG, "Pin %d read value: 0x%02x", pin, */
                        /*           value); */
                }
        return ret;
}
