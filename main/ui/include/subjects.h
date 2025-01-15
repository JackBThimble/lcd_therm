#pragma once

#include "lvgl.h"

// Subjects
extern lv_subject_t target_temperature_subject;
extern lv_subject_t wifi_status_subject;
extern lv_subject_t indoor_temperature_subject;
extern lv_subject_t indoor_humidity_subject;
extern lv_subject_t outdoor_temperature_subject;
extern lv_subject_t outdoor_humidity_subject;
extern lv_subject_t time_subject;
extern lv_subject_t date_subject;
extern lv_subject_t weather_icon_subject;
extern lv_subject_t weather_conditions_subject;

void init_subjects(void);
