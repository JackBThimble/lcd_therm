#pragma once

#include "lvgl.h"

typedef struct {
        lv_obj_t *scr;
        lv_obj_t *header_label;
        lv_obj_t *back_button;
        lv_obj_t *back_button_label;
        lv_obj_t *wifi_item;
        lv_obj_t *location_item;
        lv_obj_t *app_item;
        lv_obj_t *item_container;
} settings_screen_t;

typedef enum {
        WIFI_CB_TRIGGER,
        LOCATION_CB_TRIGGER,
        APP_CB_TRIGGER,
} settings_item_trigger_type_t;

void create_settings_screen(settings_screen_t *ss);
void settings_select_cb(lv_event_t *e);
