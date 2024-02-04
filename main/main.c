#include "esp_log.h"
#include "esp_system.h"
//#include "esp_intr_alloc.h"
#include "driver/gpio.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_pthread.h"

#include "microphone.h"
#include "speaker.h"
#include "WiFi.h"
#include "wit_ai.h"

#include "config.h"
#include "WavData.h"

static const char *TAG = "audio_recorder";
static void *AI_thread(void * arg);

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
    pthread_attr_t attr;
    pthread_t thread1;
    esp_pthread_cfg_t esp_pthread_cfg;
    int res;

    wifi_init(TAG, ESP_WIFI_SSID, ESP_WIFI_PASS);
    configure_led();

    res = pthread_create(&thread1, NULL, AI_thread, NULL);
    assert(res == 0);
    


    
}

static void *AI_thread(void * arg){
    while(true){
    
    speaker_init(TAG, 44100, I2S_NUM_TX);
    microphone_init(TAG, I2S_SAMPLE_RATE, I2S_NUM_RX);
    
    uint8_t *buffer = (uint8_t *)malloc(I2S_BUFFER_SIZE);
    if (!buffer) {
        ESP_LOGE(TAG, "Memory allocation error");
        return NULL;
    }

    speaker_play(startData, 78630);
    speaker_stop();

    gpio_set_level(BLINK_GPIO, 1);

    microphone_record(buffer, I2S_BUFFER_SIZE);

    gpio_set_level(BLINK_GPIO, 0);

    

    wit_ai_send_audio(TAG, buffer, I2S_BUFFER_SIZE, BLINK_GPIO2, AI_API_URL, AI_API_KEY, AI_API_TYPE);
    gpio_set_level(BLINK_GPIO2, 0);

    speaker_init(TAG, 8000, I2S_NUM_TX);
    speaker_play(buffer, I2S_BUFFER_SIZE);
    speaker_stop(); 

    // Release resources
    free(buffer);
    microphone_stop();
    }
}