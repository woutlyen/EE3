/**
* Author: Wout Lyen
* Team: HOME3
*/

#include "wit_ai.h"

#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "driver/gpio.h"

#include "normal_light.h"
#include "MQTT.h"

static const char *TAG = NULL;
gpio_num_t LEDPIN;

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Process and handle the response data as needed
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // You can process the response data here
            if (evt->data_len > 0) {
                ESP_LOGI(TAG, "Response data: %.*s", evt->data_len, (char *)evt->data);
                if (strstr((char *)evt->data, "Unlock") != NULL){
                    //turn_on_normal_light();
                    //gpio_set_level(LEDPIN, 1);
                    /*
                    uint8_t mes[5];
                    memset(mes, 0, sizeof(mes));
                    NRF24_t dev = get_dev();

                    mes[0] = 8;
                    mes[1] = 2;

                    Nrf24_send(&dev, mes);

                    ESP_LOGI("NRF", "Wait for sending.....");
                    if (Nrf24_isSend(&dev, 10000)) {
                        ESP_LOGI("NRF","Send success:%s", mes);
                    } else {
                        ESP_LOGW("NRF","Send fail:");
                    }
                    vTaskDelay(1);*/
                    unlock_door();

                }
                else if (strstr((char *)evt->data, "unlock") != NULL){
                    //turn_on_normal_light();
                    //gpio_set_level(LEDPIN, 1);
                    /*
                    uint8_t mes[5];
                    memset(mes, 0, sizeof(mes));
                    NRF24_t dev = get_dev();

                    mes[0] = 8;
                    mes[1] = 2;

                    Nrf24_send(&dev, mes);
                    
                    ESP_LOGI("NRF", "Wait for sending.....");
                    if (Nrf24_isSend(&dev, 10000)) {
                        ESP_LOGI("NRF","Send success:%s", mes);
                    } else {
                        ESP_LOGW("NRF","Send fail:");
                    }
                    vTaskDelay(1);*/
                    unlock_door();
                }
                memset((char *) evt->data, 0, evt->data_len);
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

void wit_ai_send_audio(const char *T, uint8_t *audio_data, size_t audio_size, gpio_num_t LED, char* API_URL, char* API_KEY, char* API_TYPE) {
    
    TAG = T;
    LEDPIN = LED;

    char* FULL_KEY = malloc(strlen(API_KEY)+1+8);
    strcpy(FULL_KEY, "Bearer "); 
    strcat(FULL_KEY, API_KEY); 
    strcat(FULL_KEY, "\n");

esp_tls_cfg_t tls_cfg = {
    .skip_common_name = false,
    // Other TLS configuration settings can be added here if needed
};
    esp_http_client_config_t config = {
        .url = API_URL,
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_POST,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set the Wit.ai API key in the request header
    esp_http_client_set_header(client, "Authorization", FULL_KEY);
    // Set the content type for audio data
    esp_http_client_set_header(client, "Content-Type", API_TYPE);

    // Send the audio data in the request body
    esp_http_client_open(client, audio_size);

    int write_size = esp_http_client_write(client, (char*)audio_data, audio_size);
    if (write_size != audio_size) {
        ESP_LOGE(TAG, "Error writing audio data to the request");
        esp_http_client_cleanup(client);
        return;
    }
    else{
    ESP_LOGI(TAG, "Size: %d\n", write_size);
    }

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    free(FULL_KEY);
}