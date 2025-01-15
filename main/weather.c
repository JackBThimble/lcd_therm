#include "weather.h"
#include "cJSON.h"
#include "display.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/projdefs.h"
#include "wifi_manager.h"

#define WEATHER_LAT_DEFAULT "35.217222"
#define WEATHER_LON_DEFAULT "-81.856667"
#define WEATHER_API_KEY "583e27bc58c7b0b822435099aa000315"

static const char *TAG = "Weather";
static weather_data_t cached_weather;
static uint64_t last_update = 0;

static char *response_data = NULL;
static size_t response_len = 0;
static bool all_chunks_received = false;

static char weather_api_key[64] = WEATHER_API_KEY;
static char weather_lat[16] = WEATHER_LAT_DEFAULT;
static char weather_lon[16] = WEATHER_LON_DEFAULT;

void
weather_set_api_key (const char *api_key)
{
        strncpy (weather_api_key, api_key, sizeof (weather_api_key) - 1);
}

void
weather_set_location (const char *lat, const char *lon)
{
        strncpy (weather_lat, lat, sizeof (weather_lat) - 1);
        strncpy (weather_lon, lon, sizeof (weather_lon) - 1);
}

static char *
get_weather_url (void)
{
        static char url[256];
        snprintf (url, sizeof (url),
                  "http://api.openweathermap.org/data/2.5/"
                  "weather?lat=%s&lon=%s&appid=%s&units=imperial",
                  weather_lat, weather_lon, weather_api_key);
        ESP_LOGI (TAG, "Weather API URL: %s", url);
        return url;
}

static esp_err_t
parse_weather_json (const char *json_string, weather_data_t *weather)
{
        cJSON *root = cJSON_Parse (json_string);
        if (root == NULL)
                {
                        return ESP_FAIL;
                }

        cJSON *main = cJSON_GetObjectItem (root, "main");
        if (main)
                {
                        cJSON *temp = cJSON_GetObjectItem (main, "temp");
                        cJSON *humidity
                            = cJSON_GetObjectItem (main, "humidity");
                        if (temp && humidity)
                                {
                                        weather->temperature
                                            = temp->valuedouble;
                                        weather->humidity
                                            = humidity->valuedouble;
                                }
                }

        cJSON *weather_array = cJSON_GetObjectItem (root, "weather");
        if (weather_array && cJSON_GetArraySize (weather_array) > 0)
                {
                        cJSON *weather_item
                            = cJSON_GetArrayItem (weather_array, 0);
                        cJSON *description = cJSON_GetObjectItem (
                            weather_item, "description");
                        if (description)
                                {
                                        strncpy (weather->condition,
                                                 description->valuestring,
                                                 sizeof (weather->condition)
                                                     - 1);
                                }
                }
        cJSON_Delete (root);
        return ESP_OK;
}

static void
log_response_headers (esp_http_client_handle_t client)
{
        int headers_length = esp_http_client_fetch_headers (client);
        if (headers_length < 0)
                {
                        ESP_LOGE (TAG, "Failed to fetch headers");
                        return;
                }

        ESP_LOGI (TAG, "Response headers: ");
        const char *header_keys[] = {
                "Access-Control-Allow-Credentials",
                "Access-Control-Allow-Methods",
                "Access-Control-Allow-Origin",
                "Connection",
                "Content-Length",
                "Content-Type",
                "Date",
                "Server",
                "X-Cache-Key",
        };
        char *header_value;
        for (size_t i = 0; i < sizeof (header_keys) / sizeof (header_keys[0]);
             i++)
                {
                        if (esp_http_client_get_header (client, header_keys[i],
                                                        &header_value)
                            == ESP_OK)
                                {
                                        ESP_LOGI (TAG, "%s: %s",
                                                  header_keys[i],
                                                  header_value);
                                }
                }
}

esp_err_t
http_event_handler (esp_http_client_event_t *evt)
{
        switch (evt->event_id)
                {
                case HTTP_EVENT_ON_DATA:
                        response_data = realloc (response_data,
                                                 response_len + evt->data_len);
                        memcpy (response_data + response_len, evt->data,
                                evt->data_len);
                        response_len += evt->data_len;
                        break;
                case HTTP_EVENT_ON_FINISH:
                        all_chunks_received = true;
                        ESP_LOGI (TAG, "Received data: %s", response_data);
                        parse_weather_json (response_data, &cached_weather);
                        break;
                default:
                        break;
                }
        return ESP_OK;
}

static esp_err_t
fetch_weather (void)
{
        ESP_LOGI (TAG, "Fetching weather information");
        esp_http_client_config_t config = {
                .url = get_weather_url (),
                .method = HTTP_METHOD_GET,
                .buffer_size = 1024,
                .event_handler = http_event_handler,
        };

        esp_http_client_handle_t client = esp_http_client_init (&config);
        if (client == NULL)
                {
                        ESP_LOGE (TAG, "Failed to initialize http client");
                        return ESP_FAIL;
                }
        esp_http_client_set_header (client, "Content-Type",
                                    "application/x-www-form-urlencoded");

        esp_err_t err = esp_http_client_perform (client);
        if (err != ESP_OK)
                {
                        ESP_LOGE (TAG, "Failed to perform http request: %s",
                                  esp_err_to_name (err));
                }
        esp_http_client_cleanup (client);
        return err;
}

void
weather_update_task (void *pvParameters)
{

        while (1)
                {
                        if (wifi_manager_is_connected ())
                                {
                                        esp_err_t err = fetch_weather ();
                                        if (err == ESP_OK)
                                                {
                                                        display_update_weather (
                                                            cached_weather

                                                                .temperature,

                                                            cached_weather

                                                                .humidity);
                                                        last_update
                                                            = esp_timer_get_time ();
                                                }
                                        else
                                                {
                                                        ESP_LOGE (
                                                            TAG,
                                                            "Failed to fetch "
                                                            "weather: %s",
                                                            esp_err_to_name (
                                                                err));
                                                }
                                }
                        else
                                {
                                        vTaskDelay (pdMS_TO_TICKS (5000));
                                        continue;
                                }
                        vTaskDelay (pdMS_TO_TICKS (300000));
                }
}

esp_err_t
weather_init (void)
{
        xTaskCreate (weather_update_task, "weather_update", 4096, NULL, 5,
                     NULL);
        return ESP_OK;
}

esp_err_t
weather_get_current (weather_data_t *weather)
{
        if (last_update == 0)
                {
                        return ESP_ERR_NOT_FOUND;
                }
        memcpy (weather, &cached_weather, sizeof (weather_data_t));
        return ESP_OK;
}
