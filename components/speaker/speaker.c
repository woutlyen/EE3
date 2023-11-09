
#include "esp_log.h"
#include "esp_system.h"
#include "esp_intr_alloc.h"
#include "driver/i2s.h"

#include "config.h"

static const char *TAG = NULL;

void speaker_init(const char *T){
    TAG = T;

    i2s_config_t i2s_tx_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
    };

    i2s_pin_config_t pin_tx_config = {
        .bck_io_num = 11, // Configure these pins based on your hardware setup
        .ws_io_num = 10,
        .data_out_num = 12,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };


    esp_err_t err;

    err = i2s_driver_install(I2S_NUM_TX, &i2s_tx_config, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S TX driver install error: %d", err);
        return;
    }

    err = i2s_set_pin(I2S_NUM_TX, &pin_tx_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S TX set pin error: %d", err);
        return;
    }
}

void speaker_play(uint8_t *audio_data, size_t audio_size){
    size_t bytes_read = 0;
    esp_err_t err;

    while (bytes_read < audio_size) {
        err = i2s_write(I2S_NUM_TX, audio_data + bytes_read, audio_size - bytes_read, &bytes_read, portMAX_DELAY);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "I2S write error: %d", err);
            break;
        }
    }
}

