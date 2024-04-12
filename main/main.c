#define PIN_NUM_MISO  GPIO_NUM_7
#define PIN_NUM_MOSI  GPIO_NUM_5
#define PIN_NUM_CLK   GPIO_NUM_6
#define PIN_NUM_CS    GPIO_NUM_4 

#include "esp_log.h"
#include "esp_system.h"
//#include "esp_intr_alloc.h"
#include "driver/gpio.h"

#include <stdio.h>
#include <stdlib.h>
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
#include "SD.h"

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
//#include "WavData.h"

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#define MOUNT_POINT "/sdcard"
#define AUDIO_BUFFER 1024


// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO  GPIO_NUM_7
#define PIN_NUM_MOSI  GPIO_NUM_5
#define PIN_NUM_CLK   GPIO_NUM_6
#define PIN_NUM_CS    GPIO_NUM_4 

#define EXAMPLE_MAX_CHAR_SIZE    64

esp_mqtt_client_handle_t event_client;

static const char *TAG = "audio_recorder";
static void *AI_thread(void * arg);
static void *MQTT_LIGHT_TEMP_thread(void * arg);
static void *MUSIC_thread(void * arg);

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

    init_rgb_light();
    init_dimmable_light();
    //on_rgb_light();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    init_nrf();
    event_client = init_mqtt();
    init_motion_sensor();

    pthread_attr_t attr;
    pthread_t thread1, thread2, thread3;
    esp_pthread_cfg_t esp_pthread_cfg;
    int res;

    //wifi_init(TAG, ESP_WIFI_SSID, ESP_WIFI_PASS);
    //configure_led();

    //res = pthread_create(&thread2, NULL, MQTT_LIGHT_TEMP_thread, NULL);
    //assert(res == 0);
    res = pthread_create(&thread3, NULL, MUSIC_thread, NULL);
    assert(res == 0);

    //esp_pthread_cfg.stack_size = (16 * 1024);
    //esp_pthread_set_cfg(&esp_pthread_cfg);
    

    res = pthread_create(&thread1, NULL, AI_thread, NULL);
    assert(res == 0);


    //set_rgb_light(0,100,0,true);

    //set_rgb_light(150,30,0,true);
    //init_SD();

    start_nrf_communication();
}

static void *MUSIC_thread(void *arg){
    init_SD();
    return NULL;
}

static void *AI_thread(void * arg){

    //return NULL;

    while(true){
        vTaskDelay(1);
        if(get_voice_assistant_activated()){
            //speaker_init(TAG, 44100, I2S_NUM_TX);
            microphone_init(TAG, I2S_SAMPLE_RATE, I2S_NUM_RX);
            
            uint8_t *buffer = (uint8_t *)malloc(I2S_BUFFER_SIZE);
            if (!buffer) {
                ESP_LOGE(TAG, "Memory allocation error");
                //speaker_stop();
                //microphone_stop();
                //return NULL;
                esp_restart();
            }
            else{
                //speaker_play(startData, 78630);
                //speaker_stop();

                //on_dimmable_light();
                //gpio_set_level(BLINK_GPIO, 1);

                set_start_sound();
                while(get_start_sound()) vTaskDelay(100);
                microphone_record(buffer, I2S_BUFFER_SIZE);
                set_stop_sound();
                wit_ai_send_audio(TAG, buffer, I2S_BUFFER_SIZE, BLINK_GPIO2, AI_API_URL, AI_API_KEY, AI_API_TYPE);
                
                //while(get_stop_sound()) vTaskDelay(100);
                
                //set_dimmable_light_brightness(0, false);
                //gpio_set_level(BLINK_GPIO, 0);

                

                //turn_off_normal_light();
                //gpio_set_level(BLINK_GPIO2, 0);

                //speaker_init(TAG, 8000, I2S_NUM_TX);
                //speaker_play(buffer, I2S_BUFFER_SIZE);
                //speaker_stop(); 

                // Release resources
                free(buffer);
                microphone_stop();
                set_voice_assistant_deactivated();
            }
        }
    }
    return NULL;
}

static void *MQTT_LIGHT_TEMP_thread(void * arg){

    return NULL;
    
    init_rgb_light();
    init_dimmable_light();
    init_normal_light();
    event_client = init_mqtt();
    init_motion_sensor();
    start_heating_temp_sensor();

    return NULL;
}