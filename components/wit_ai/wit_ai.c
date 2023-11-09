#include "wit_ai.h"
#include "config.h"

#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "driver/gpio.h"

static const char *TAG = NULL;

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Process and handle the response data as needed
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // You can process the response data here
            if (evt->data_len > 0) {
                ESP_LOGI(TAG, "Response data: %.*s", evt->data_len, (char *)evt->data);
                if (strstr((char *)evt->data, "Unlock") != NULL){
                    gpio_set_level(BLINK_GPIO2, 1);
                }
                else if (strstr((char *)evt->data, "unlock") != NULL){
                    gpio_set_level(BLINK_GPIO2, 1);
                }
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}



void wit_ai_send_audio(const char *T, uint8_t *audio_data, size_t audio_size) {
    
    TAG = T;
    
    esp_tls_cfg_t tls_cfg = {
    .skip_common_name = false,
    // Other TLS configuration settings can be added here if needed
};
    esp_http_client_config_t config = {
        .url = "https://api.wit.ai/dictation?v=20231106",
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_POST,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set the Wit.ai API key in the request header
    esp_http_client_set_header(client, "Authorization", "Bearer " WITAI_API_KEY "\n");
    // Set the content type for audio data
    esp_http_client_set_header(client, "Content-Type", "audio/raw;encoding=signed-integer;bits=16;rate=8000;endian=little");

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
}