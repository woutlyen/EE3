#include "esp_log.h"
#include "esp_system.h"
//#include "esp_intr_alloc.h"
#include "driver/i2s.h"
#include "driver/gpio.h"


#include "WiFi.h"
#include "speaker.h"
#include "microphone.h"
#include "wit_ai.h"
#include "config.h"

static const char *TAG = "audio_recorder";


static void configure_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_reset_pin(BLINK_GPIO2);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO2, GPIO_MODE_OUTPUT);
}


void app_main(void)
{

    wifi_init(TAG);
    configure_led();

    while(true){
    
    speaker_init(TAG, I2S_SAMPLE_RATE);
    microphone_init(TAG, I2S_SAMPLE_RATE);
    
    uint8_t *buffer = (uint8_t *)malloc(I2S_BUFFER_SIZE);
    if (!buffer) {
        ESP_LOGE(TAG, "Memory allocation error");
        return;
    }

    gpio_set_level(BLINK_GPIO, 1);
    microphone_record(buffer, I2S_BUFFER_SIZE);
    gpio_set_level(BLINK_GPIO, 0);
    wit_ai_send_audio(TAG, buffer, I2S_BUFFER_SIZE, BLINK_GPIO2);
    speaker_play(buffer, I2S_BUFFER_SIZE);
    gpio_set_level(BLINK_GPIO2, 0);


    // Release resources
    free(buffer);
    i2s_driver_uninstall(I2S_NUM_RX);
    i2s_driver_uninstall(I2S_NUM_TX);
    }
}
