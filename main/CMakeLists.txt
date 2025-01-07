idf_component_register(
                        SRCS
                          "main.c"
                          "lv_port.c"
                          "ui.c"
                          "expander.c"
                          "i2c_bus.c"
                        INCLUDE_DIRS "./include"
                        REQUIRES
                          esp_common
                          log
                          nvs_flash
                          esp_wifi
                          esp_http_client
                          esp_driver_i2c
                          esp_lcd
                          esp_driver_gpio
                          esp_event
                          esp_netif
)

