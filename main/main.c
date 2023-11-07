#include "esp_log.h"
#include "esp_system.h"
#include "esp_intr_alloc.h"
#include "driver/i2s.h"
#include "esp_http_client.h"
#include "esp_tls.h"

#include <string.h> //for handling strings
#include "freertos/FreeRTOS.h" //for delay,mutexs,semphrs rtos operations
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h" //esp_init funtions esp_err_t 
#include "esp_wifi.h" //esp_wifi_init functions and wifi operations
#include "esp_log.h" //for showing logs
#include "esp_event.h" //for wifi event
#include "nvs_flash.h" //non volatile storage
#include "lwip/err.h" //light weight ip packets error handling
#include "lwip/sys.h" //system applications for light weight ip apps

#include "driver/gpio.h"

#define BLINK_GPIO GPIO_NUM_37

static const char *TAG = "audio_recorder";

#define I2S_NUM_RX  I2S_NUM_0
#define I2S_NUM_TX  I2S_NUM_1
#define I2S_SAMPLE_RATE (8000)
#define I2S_BUFFER_SIZE (I2S_SAMPLE_RATE * 6)
#define WITAI_API_KEY "V55E3FX2CUEA4XPZXJKULRZ2VLCZD752"





#define EXAMPLE_ESP_WIFI_SSID      "Galaxy-S20+e7ca"
#define EXAMPLE_ESP_WIFI_PASS      "jcec9528"
#define EXAMPLE_ESP_MAXIMUM_RETRY  10

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}








static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Process and handle the response data as needed
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // You can process the response data here
            if (evt->data_len > 0) {
                ESP_LOGI(TAG, "Response data: %.*s", evt->data_len, (char *)evt->data);
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

void send_audio_to_witai(uint8_t *audio_data, size_t audio_size) {
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



static void configure_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}


void app_main(void)
{

//Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();
    configure_led();

    while(true){
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

    i2s_pin_config_t pin_rx_config = {
        .bck_io_num = 16,
        .ws_io_num = 17,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 15,
    };

    i2s_pin_config_t pin_tx_config = {
        .bck_io_num = 11, // Configure these pins based on your hardware setup
        .ws_io_num = 10,
        .data_out_num = 12,
        .data_in_num = I2S_PIN_NO_CHANGE,
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

    send_audio_to_witai(buffer, I2S_BUFFER_SIZE);
    bytes_read = 0;
    
    
    while (bytes_read < I2S_BUFFER_SIZE) {
        err = i2s_write(I2S_NUM_TX, buffer + bytes_read, I2S_BUFFER_SIZE - bytes_read, &bytes_read, portMAX_DELAY);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "I2S write error: %d", err);
            break;
        }
    }
    


    // Release resources
    free(buffer);
    i2s_driver_uninstall(I2S_NUM_RX);
    i2s_driver_uninstall(I2S_NUM_TX);
    }
}
