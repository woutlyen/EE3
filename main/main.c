#include "esp_log.h"
#include "esp_system.h"
//#include "esp_intr_alloc.h"
#include "driver/gpio.h"


#include "WiFi.h"
#include "speaker.h"
#include "microphone.h"
#include "wit_ai.h"
#include "config.h"

static const char *TAG = "audio_recorder";


void configure_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_reset_pin(BLINK_GPIO2);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO2, GPIO_MODE_OUTPUT);
}


void app_main(void)
{

    wifi_init(TAG, ESP_WIFI_SSID, ESP_WIFI_PASS);
    configure_led();

    while(true){
    
    speaker_init(TAG, I2S_SAMPLE_RATE, I2S_NUM_TX);
    microphone_init(TAG, I2S_SAMPLE_RATE, I2S_NUM_RX);
    
    uint8_t *buffer = (uint8_t *)malloc(I2S_BUFFER_SIZE);
    if (!buffer) {
        ESP_LOGE(TAG, "Memory allocation error");
        return;
    }

    gpio_set_level(BLINK_GPIO, 1);
    microphone_record(buffer, I2S_BUFFER_SIZE);
    gpio_set_level(BLINK_GPIO, 0);
    wit_ai_send_audio(TAG, buffer, I2S_BUFFER_SIZE, BLINK_GPIO2, AI_API_URL, AI_API_KEY, AI_API_TYPE);
    speaker_play(buffer, I2S_BUFFER_SIZE);
    gpio_set_level(BLINK_GPIO2, 0);


    // Release resources
    free(buffer);
    speaker_stop();
    microphone_stop();
    }
}
