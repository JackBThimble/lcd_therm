#include "rtc.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/i2c_types.h"
#include "i2c_bus.h"
#include <string.h>

#define RTC_ADDR 0x51
#define RTC_REG_CTRL1 0x00
#define RTC_REG_CTRL2 0x01
#define RTC_REG_OS 0x02
#define RTC_REG_RAM 0x03
#define RTC_REG_SECONDS 0x04
#define RTC_REG_MINUTES 0x05
#define RTC_REG_HOURS 0x06
#define RTC_REG_DAYS 0x07
#define RTC_REG_WEEKDAYS 0x08
#define RTC_REG_MONTHS 0x09
#define RTC_REG_YEARS 0x0a
#define RTC_REG_SEC_ALRM 0x0b
#define RTC_REG_MIN_ALRM 0x0c
#define RTC_REG_HR_ALRM 0x0d
#define RTC_REG_DAY_ALRM 0x0e
#define RTC_REG_WKDY_ALRM 0x0f
#define RTC_REG_TIMER_VAL 0x10
#define RTC_REG_TIMER_MODE 0x11

static const char *TAG = "rtc";
static i2c_master_dev_handle_t rtc_handle;

static uint8_t dec_to_bcd(uint8_t dec) {
        return ((dec / 10) << 4) | (dec % 10);
}

static uint8_t bcd_to_dec(uint8_t bcd) {
        return ((bcd >> 4) * 10) + (bcd & 0x0f);
}

static esp_err_t rtc_read_registers(uint8_t reg_addr, uint8_t *data,
                                    size_t len) {
        uint8_t write_buf = reg_addr;
        esp_err_t ret = i2c_master_transmit(rtc_handle, &write_buf, 1, -1);
        if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to tramsit RTC command: %s",
                         esp_err_to_name(ret));
                return ret;
        }
        ret = i2c_master_receive(rtc_handle, data, len, -1);
        if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to receive RTC registers: %s",
                         esp_err_to_name(ret));
                return ret;
        }
        return ret;
}

static esp_err_t rtc_write_registers(uint8_t reg_addr, uint8_t *data,
                                     size_t len) {
        uint8_t *write_buf = malloc(len + 1);
        if (write_buf == NULL) {
                ESP_LOGE(TAG, "Failed to allocate memory for RTC write buffer");
                return ESP_ERR_NO_MEM;
        }

        write_buf[0] = reg_addr;
        memcpy(&write_buf[1], data, len);

        esp_err_t ret = i2c_master_transmit(rtc_handle, write_buf, len + 1, -1);
        free(write_buf);
        if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to transmit RTC command: %s",
                         esp_err_to_name(ret));
                return ret;
        }
        return ret;
}
esp_err_t ext_rtc_init(void) {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = RTC_ADDR,
            .scl_speed_hz = 400000,
        };

        esp_err_t ret =
            i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &rtc_handle);
        if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to add RTC device to the I2C bus: %s",
                         esp_err_to_name(ret));
                return ret;
        }

        uint8_t ctrl1 = 0x00; // Normal mode, clock running
        uint8_t ctrl2 = 0x00; // No alarms or timer

        if ((ret = rtc_write_registers(RTC_REG_CTRL1, &ctrl1, 1)) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to write CTRL1: %s",
                         esp_err_to_name(ret));
                return ret;
        }
        if ((ret = rtc_write_registers(RTC_REG_CTRL2, &ctrl2, 1)) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to write CTLR2: %s",
                         esp_err_to_name(ret));
                return ret;
        }
        return ESP_OK;
}

esp_err_t rtc_set_time(const struct tm *time) {
        uint8_t time_data[7];
        time_data[0] = dec_to_bcd(time->tm_sec);
        time_data[1] = dec_to_bcd(time->tm_min);
        time_data[2] = dec_to_bcd(time->tm_hour);
        time_data[3] = dec_to_bcd(time->tm_mday);
        time_data[4] = dec_to_bcd(time->tm_wday);
        time_data[5] = dec_to_bcd(time->tm_mon + 1);
        time_data[6] = dec_to_bcd(time->tm_year % 100);

        return rtc_write_registers(RTC_REG_SECONDS, time_data,
                                   sizeof(time_data));
}

esp_err_t rtc_get_time(struct tm *time) {
        uint8_t time_data[7];
        esp_err_t ret;
        if ((ret = rtc_read_registers(RTC_REG_SECONDS, time_data,
                                      sizeof(time_data))) != ESP_OK)
                return ret;

        time->tm_sec = bcd_to_dec(time_data[0] & 0x7f);
        time->tm_min = bcd_to_dec(time_data[1] & 0x7f);
        time->tm_hour = bcd_to_dec(time_data[2] & 0x3f);
        time->tm_mday = bcd_to_dec(time_data[3] & 0x3f);
        time->tm_wday = bcd_to_dec(time_data[4] & 0x07);
        time->tm_mon = bcd_to_dec(time_data[5] & 0x1f) - 1;
        time->tm_year = bcd_to_dec(time_data[6]) + 100;

        return ESP_OK;
}
