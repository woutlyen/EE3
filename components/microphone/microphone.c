#include "microphone.h"
#include "config.h"

#include "esp_log.h"
#include "esp_system.h"
//#include "esp_intr_alloc.h"

static const char *TAG = NULL;
i2s_port_t PORT_RX;

void microphone_init(const char *T, size_t sample_rate, i2s_port_t PORT){

    TAG = T;
    PORT_RX = PORT;

    i2s_config_t i2s_rx_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = sample_rate,
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

    err = i2s_driver_install(PORT_RX, &i2s_rx_config, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S RX driver install error: %d", err);
        return;
    }

    err = i2s_set_pin(PORT_RX, &pin_rx_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S RX set pin error: %d", err);
        return;
    }
}

void microphone_record(uint8_t *audio_data, size_t audio_size){
    size_t bytes_read = 0;
    esp_err_t err;

    while (bytes_read < audio_size) {
        err = i2s_read(PORT_RX, audio_data + bytes_read, audio_size - bytes_read, &bytes_read, portMAX_DELAY);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "I2S read error: %d", err);
            break;
        }
    }
}