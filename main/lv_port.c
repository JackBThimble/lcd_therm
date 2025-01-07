#include "lv_port.h"
#include "display/lv_display.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "expander.h"
#include "i2c_bus.h"
#include "lcd.h"
#include "lvgl.h"

#include "esp_lcd_touch_gt911.h"
#include "soc/clk_tree_defs.h"

static const char *TAG = "LVGL Port";
static esp_lcd_panel_handle_t lcd_panel = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;

void gt911_reset(void) {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
        io_conf.mode = GPIO_MODE_OUTPUT;

        gpio_config(&io_conf);

        expander_set_level(1, 0);
        esp_rom_delay_us(100 * 1000);
        gpio_set_level(GPIO_INPUT_IO_4, 0);
        esp_rom_delay_us(100 * 1000);
        expander_set_level(1, 1);
        esp_rom_delay_us(200 * 1000);

        io_conf.intr_type = GPIO_INTR_NEGEDGE;
        io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
        io_conf.mode = GPIO_MODE_INPUT;
}

esp_err_t lcd_init(void) {
        esp_err_t ret = ESP_OK;

        ESP_LOGI(TAG, "Initialize RGB Panel");
        esp_lcd_rgb_panel_config_t panel_conf = {
            .clk_src = LCD_CLK_SRC_PLL240M,
            .data_width = 16,
            .bits_per_pixel = 16,
            .de_gpio_num = LCD_IO_RGB_DE,
            .pclk_gpio_num = LCD_IO_RGB_PCLK,
            .vsync_gpio_num = LCD_IO_RGB_VSYNC,
            .hsync_gpio_num = LCD_IO_RGB_HSYNC,
            .disp_gpio_num = LCD_IO_RGB_DISP,
            .data_gpio_nums =
                {
                    LCD_IO_RGB_DATA0,
                    LCD_IO_RGB_DATA1,
                    LCD_IO_RGB_DATA2,
                    LCD_IO_RGB_DATA3,
                    LCD_IO_RGB_DATA4,
                    LCD_IO_RGB_DATA5,
                    LCD_IO_RGB_DATA6,
                    LCD_IO_RGB_DATA7,
                    LCD_IO_RGB_DATA8,
                    LCD_IO_RGB_DATA9,
                    LCD_IO_RGB_DATA10,
                    LCD_IO_RGB_DATA11,
                    LCD_IO_RGB_DATA12,
                    LCD_IO_RGB_DATA13,
                    LCD_IO_RGB_DATA14,
                    LCD_IO_RGB_DATA15,
                },
            .timings =
                {
                    .pclk_hz = 16 * 1000 * 1000,
                    .h_res = LCD_H_RES,
                    .v_res = LCD_V_RES,
                    .hsync_pulse_width = 4,
                    .hsync_back_porch = 8,
                    .hsync_front_porch = 8,
                    .vsync_pulse_width = 4,
                    .vsync_back_porch = 8,
                    .vsync_front_porch = 8,
                    .flags =
                        {
                            .pclk_active_neg = 1,
                        },
                },
            .flags =
                {
                    .fb_in_psram = 1,
                },
            .num_fbs = 2,
        };
        if ((ret = esp_lcd_new_rgb_panel(&panel_conf, &lcd_panel)) != ESP_OK) {
                ESP_LOGE(TAG, "RBG LCD init failed");
                return ret;
        }

        if ((ret = esp_lcd_panel_init(lcd_panel)) != ESP_OK) {
                ESP_LOGE(TAG, "LCD init failed");
                return ret;
        }
        return ESP_OK;
}

esp_err_t touch_init(void) {
        esp_err_t ret;
        esp_lcd_panel_io_i2c_config_t io_config =
            ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
        io_config.dev_addr = 0x5d;
        io_config.scl_speed_hz = 400000;

        esp_lcd_touch_io_gt911_config_t tp_gt911_config = {
            .dev_addr = io_config.dev_addr};

        esp_lcd_touch_config_t tp_cfg = {
            .y_max = LCD_V_RES,
            .x_max = LCD_H_RES,
            .rst_gpio_num = -1,
            .int_gpio_num = 4,
            .levels =
                {
                    .reset = 0,
                    .interrupt = 0,
                },
            .flags =
                {
                    .swap_xy = 0,
                    .mirror_x = 0,
                    .mirror_y = 0,
                },
            .driver_data = &tp_gt911_config,
        };
        esp_lcd_panel_io_handle_t tp_io_handle = NULL;
        gt911_reset();
        if ((ret = esp_lcd_new_panel_io_i2c(i2c_bus_handle, &io_config,
                                            &tp_io_handle)) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to add new panel IO to i2c bus");
                return ret;
        }
        if ((esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg,
                                         &touch_handle)) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to initialize GT911 Touch controller");
                return ret;
        }
        return ESP_OK;
}

esp_err_t lvgl_init(void) {
        const lvgl_port_cfg_t lvgl_cfg = {
            .task_priority = 4,
            .task_stack = 8192,
            .task_affinity = 1,
            .task_max_sleep_ms = 500,
            .timer_period_ms = 5,
        };

        esp_err_t ret;
        if ((ret = lvgl_port_init(&lvgl_cfg)) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to initialize LVGL port");
                return ret;
        }
        uint32_t buff_size = LCD_H_RES * LCD_V_RES;

        ESP_LOGD(TAG, "Add LCD Screen");
        const lvgl_port_display_cfg_t disp_cfg = {.panel_handle = lcd_panel,
                                                  .buffer_size = buff_size,
                                                  .double_buffer = true,
                                                  .hres = LCD_H_RES,
                                                  .vres = LCD_V_RES,
                                                  .monochrome = false,
                                                  .color_format =
                                                      LV_COLOR_FORMAT_RGB565,
                                                  .rotation =
                                                      {
                                                          .mirror_x = false,
                                                          .mirror_y = false,
                                                          .swap_xy = false,
                                                      },
                                                  .flags = {
                                                      .buff_dma = true,
                                                      .buff_spiram = true,
                                                      .direct_mode = false,
                                                      .swap_bytes = false,
                                                  }};

        const lvgl_port_display_rgb_cfg_t rgb_cfg = {.flags = {
                                                         .avoid_tearing = true,
                                                         .bb_mode = false,
                                                     }};

        lvgl_disp = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lvgl_disp,
            .handle = touch_handle,
        };
        lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);

        return ESP_OK;
}
