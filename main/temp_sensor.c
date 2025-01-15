#include "temp_sensor.h"
#include "app_settings.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"
#include "lvgl.h"
#include "main.h"
#include "soc/clk_tree_defs.h"
#include "subjects.h"
#include "ui.h"

#define SHT3X_ADDR 0x44
#define SHT3X_HIGH_PRECISION_SINGLE 0x2400
#define SHT3X_MEDIUM_PRECISION 0x240B
#define SHT3X_LOW_PRECISION 0x2416
#define CRC8_POLY 0x31
#define CRC8_INIT 0xff

static const char *TAG = "Temperature Sensor";
static i2c_master_dev_handle_t sensor_handle;

static void update_temp_sensor_task(void *pvParameters) {
        esp_task_wdt_config_t wdt_cfg = {
            .timeout_ms = 10000,
        };
        esp_task_wdt_reconfigure(&wdt_cfg);
        esp_task_wdt_add(NULL);
        float temperature, humidity;
        esp_err_t err;
        uint8_t cmd[2] = {(SHT3X_HIGH_PRECISION_SINGLE >> 8),
                          (SHT3X_HIGH_PRECISION_SINGLE & 0xff)};

        while (1) {
                esp_task_wdt_reset();
                /* ESP_LOGI (TAG, "Transmitting SHT3x command"); */
                if ((err = i2c_master_transmit(sensor_handle, cmd, 2, -1)) ==
                    ESP_OK) {
                        /* ESP_LOGI (TAG, */
                        /*           "SHT3x command transmitted
                         * " */
                        /*           "successfully"); */
                        vTaskDelay(pdMS_TO_TICKS(20));
                        if ((err = temp_sensor_read(&temperature, &humidity)) ==
                            ESP_OK) {
                                g_indoor_temperature = temperature;
                                g_indoor_humidity = humidity;
                                lvgl_port_lock(0);
                                lv_subject_set_int(&indoor_temperature_subject,
                                                   g_indoor_temperature);
                                lv_subject_notify(&indoor_temperature_subject);
                                lv_subject_set_int(&indoor_humidity_subject,
                                                   g_indoor_humidity);
                                lv_subject_notify(&indoor_humidity_subject);
                                lvgl_port_unlock();
                        }
                } else {
                        ESP_LOGE(TAG,
                                 "Failed to transmit "
                                 "SHT3x command: %s",
                                 esp_err_to_name(err));
                }
                vTaskDelay(pdMS_TO_TICKS(2000)); // Update every 2 seconds
        }
}

static uint8_t calculate_crc8(const uint8_t *data, size_t len) {
        uint8_t crc = CRC8_INIT;
        for (size_t i = 0; i < len; i++) {
                crc ^= data[i];
                for (size_t j = 0; j < 8; j++) {
                        if (crc & 0x80) {
                                crc = (crc << 1) ^ CRC8_POLY;
                        } else {
                                crc = crc << 1;
                        }
                }
        }
        return crc;
}

esp_err_t temp_sensor_init(void) {
        esp_err_t ret;
        i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = SHT3X_ADDR,
            .scl_speed_hz = 400000,
        };
        /* ESP_LOGI (TAG, "Adding SHT3x to I2C bus"); */
        if ((ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_config,
                                             &sensor_handle)) != ESP_OK) {
                ESP_LOGE(TAG, "Adding SHT3x to i2c bus failed: %s",
                         esp_err_to_name(ret));
                return ret;
        }
        /* ESP_LOGI (TAG, "SHT3x initialization successful"); */

        /* ESP_LOGI (TAG, "Launching 'sensor_update' task"); */
        xTaskCreate(update_temp_sensor_task, "temp_sensor_task",
                    TEMP_TASK_STACK_SIZE, NULL, TEMP_TASK_PRIORITY, NULL);
        return ESP_OK;
}

esp_err_t temp_sensor_read(float *temperature, float *humidity) {
        esp_err_t ret;
        uint8_t data[6];

        /* ESP_LOGI (TAG, "Receiving SHT3x data"); */
        if ((ret = i2c_master_receive(sensor_handle, data, 6, -1)) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to read data: %s", esp_err_to_name(ret));
                return ret;
        }
        /* ESP_LOGI (TAG, "SHT3x data received successfully"); */

        /* ESP_LOGI (TAG, "Verifying temperature data checksum"); */
        uint8_t temp_crc = calculate_crc8(data, 2);
        if (temp_crc != data[2]) {
                ESP_LOGE(TAG,
                         "Temperature CRC mismatch: calculated "
                         "0x%02x, received 0x%02x",
                         temp_crc, data[2]);
                return ESP_ERR_INVALID_CRC;
        }
        /* ESP_LOGI (TAG, "Temperature data checksum verified successfully");
         */

        /* ESP_LOGI (TAG, "Verifying humidity data checksum"); */
        uint8_t hum_crc = calculate_crc8(&data[3], 2);
        if (hum_crc != data[5]) {
                ESP_LOGE(TAG,
                         "Humiidity CRC mismatch: calculated 0x%02x, "
                         "received 0x%02x",
                         hum_crc, data[5]);
                return ESP_ERR_INVALID_CRC;
        }
        /* ESP_LOGI (TAG, "Humidity data checksum verified successfully"); */

        uint16_t temp_raw = (data[0] << 8) | data[1];
        uint16_t hum_raw = (data[3] << 8) | data[4];

        /* ESP_LOGI (TAG, "Temperature ticks: %d", temp_raw); */
        /* ESP_LOGI (TAG, "Humidity ticks: %d", hum_raw); */

        *temperature = ((float)temp_raw * 315.0f / 65535.0f) - 49.0f;
        *humidity = ((float)hum_raw * 125.0f / 65535.0f);

        ESP_LOGI(TAG, "Temperature: %.02f F", *temperature);
        ESP_LOGI(TAG, "Humidity: %.02f%%", *humidity);
        return ESP_OK;
}
