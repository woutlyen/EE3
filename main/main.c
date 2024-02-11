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
//#include "WiFi.h"
#include "wit_ai.h"
#include "heating_temp_sensor.h"
#include "RGB_light.h"
#include "dimmable_light.h"
#include "normal_light.h"
#include "motion_sensor.h"
#include "MQTT.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "mqtt_client.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "protocol_examples_common.h"

#include "config.h"
#include "WavData.h"

esp_mqtt_client_handle_t event_client;

static const char *TAG = "audio_recorder";
static void *AI_thread(void * arg);
static void *MQTT_LIGHT_TEMP_thread(void * arg);

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
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    pthread_attr_t attr;
    pthread_t thread1, thread2;
    esp_pthread_cfg_t esp_pthread_cfg;
    int res;

    //wifi_init(TAG, ESP_WIFI_SSID, ESP_WIFI_PASS);
    //configure_led();

    res = pthread_create(&thread1, NULL, AI_thread, NULL);
    assert(res == 0);
    res = pthread_create(&thread2, NULL, MQTT_LIGHT_TEMP_thread, NULL);
    assert(res == 0);
    


    
}

static void *AI_thread(void * arg){
    while(true){
    
    speaker_init(TAG, 44100, I2S_NUM_TX);
    microphone_init(TAG, I2S_SAMPLE_RATE, I2S_NUM_RX);
    
    uint8_t *buffer = (uint8_t *)malloc(I2S_BUFFER_SIZE);
    if (!buffer) {
        ESP_LOGE(TAG, "Memory allocation error");

        speaker_stop();
        microphone_stop();
        //return NULL;
    }
    else{
        speaker_play(startData, 78630);
        speaker_stop();

        on_dimmable_light();
        //gpio_set_level(BLINK_GPIO, 1);

        microphone_record(buffer, I2S_BUFFER_SIZE);

        set_dimmable_light_brightness(0, false);
        //gpio_set_level(BLINK_GPIO, 0);

        

        wit_ai_send_audio(TAG, buffer, I2S_BUFFER_SIZE, BLINK_GPIO2, AI_API_URL, AI_API_KEY, AI_API_TYPE);
        turn_off_normal_light();
        //gpio_set_level(BLINK_GPIO2, 0);

        speaker_init(TAG, 8000, I2S_NUM_TX);
        speaker_play(buffer, I2S_BUFFER_SIZE);
        speaker_stop(); 

        // Release resources
        free(buffer);
        microphone_stop();
        }
    }
    return NULL;
}

static void *MQTT_LIGHT_TEMP_thread(void * arg){
    init_rgb_light();
    init_dimmable_light();
    init_normal_light();
    event_client = init_mqtt();
    init_motion_sensor();
    start_heating_temp_sensor();

    return NULL;
}