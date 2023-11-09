#include "esp_log.h"
#include "esp_system.h"
#include "esp_intr_alloc.h"
#include "driver/i2s.h"
#include "driver/gpio.h"


#include "WiFi.h"
#include "speaker.h"
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
    
    speaker_init(TAG);

    i2s_config_t i2s_rx_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
    };

    i2s_pin_config_t pin_rx_config = {
        .bck_io_num = 16,
        .ws_io_num = 17,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 15,
    };

    esp_err_t err;

    err = i2s_driver_install(I2S_NUM_RX, &i2s_rx_config, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S RX driver install error: %d", err);
        return;
    }

    err = i2s_set_pin(I2S_NUM_RX, &pin_rx_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S RX set pin error: %d", err);
        return;
    }


    size_t bytes_read = 0;
    uint8_t *buffer = (uint8_t *)malloc(I2S_BUFFER_SIZE);
    if (!buffer) {
        ESP_LOGE(TAG, "Memory allocation error");
        return;
    }

    gpio_set_level(BLINK_GPIO, 1);
    while (bytes_read < I2S_BUFFER_SIZE) {
        err = i2s_read(I2S_NUM_RX, buffer + bytes_read, I2S_BUFFER_SIZE - bytes_read, &bytes_read, portMAX_DELAY);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "I2S read error: %d", err);
            break;
        }
    }


    gpio_set_level(BLINK_GPIO, 0);
    wit_ai_send_audio(TAG, buffer, I2S_BUFFER_SIZE);
    speaker_play(buffer, I2S_BUFFER_SIZE);
    gpio_set_level(BLINK_GPIO2, 0);


    // Release resources
    free(buffer);
    i2s_driver_uninstall(I2S_NUM_RX);
    i2s_driver_uninstall(I2S_NUM_TX);
    }
}
