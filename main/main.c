#define PIN_NUM_MISO  GPIO_NUM_7
#define PIN_NUM_MOSI  GPIO_NUM_5
#define PIN_NUM_CLK   GPIO_NUM_6
#define PIN_NUM_CS    GPIO_NUM_4 

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

void configure_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_reset_pin(BLINK_GPIO2);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO2, GPIO_MODE_OUTPUT);
}


static esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

static esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}

static esp_err_t play_wav(const char *path)
{
  FILE *fh = fopen(path, "rb");
  if (fh == NULL)
  {
    ESP_LOGE(TAG, "Failed to open file");
    return ESP_ERR_INVALID_ARG;
  }

  // skip the header...
  fseek(fh, 44, SEEK_SET);

  // create a writer buffer
  uint8_t *buf = calloc(AUDIO_BUFFER, sizeof(int8_t));
  size_t bytes_read = 0;
  size_t bytes_written = 0;

  bytes_read = fread(buf, sizeof(int8_t), AUDIO_BUFFER, fh);

  speaker_init(TAG, 48000, I2S_NUM_TX);

  while (bytes_read > 0)
  {
    speaker_play(buf, AUDIO_BUFFER);
    // write the buffer to the i2s
    //i2s_channel_write(tx_handle, buf, bytes_read * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    bytes_read = fread(buf, sizeof(int8_t), AUDIO_BUFFER, fh);
    ESP_LOGV(TAG, "Bytes read: %d", bytes_read);
  }

  //i2s_channel_disable(tx_handle);
  speaker_stop();
  free(buf);
  fclose(fh);
  return ESP_OK;
}


void app_main(void)
{
    init_rgb_light();
    on_rgb_light();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    event_client = init_mqtt();

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
    
    set_rgb_light(0,100,0,true);

    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    ESP_LOGI(TAG, "Using SPI peripheral");

    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 20MHz for SDSPI)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files.

    // First create a file.
    const char *file_hello = MOUNT_POINT"/hello.txt";
    char data[EXAMPLE_MAX_CHAR_SIZE];
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Hello", card->cid.name);
    ret = s_example_write_file(file_hello, data);
    if (ret != ESP_OK) {
        return;
    }

    const char *file_foo = MOUNT_POINT"/foo.txt";

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_foo, &st) == 0) {
        // Delete it if it exists
        unlink(file_foo);
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    ret = s_example_read_file(file_foo);
    if (ret != ESP_OK) {
        return;
    }

    // Format FATFS
#ifdef CONFIG_EXAMPLE_FORMAT_SD_CARD
    ret = esp_vfs_fat_sdcard_format(mount_point, card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to format FATFS (%s)", esp_err_to_name(ret));
        return;
    }

    if (stat(file_foo, &st) == 0) {
        ESP_LOGI(TAG, "file still exists");
        return;
    } else {
        ESP_LOGI(TAG, "file doesnt exist, format done");
    }
#endif // CONFIG_EXAMPLE_FORMAT_SD_CARD

    const char *file_nihao = MOUNT_POINT"/nihao.txt";
    memset(data, 0, EXAMPLE_MAX_CHAR_SIZE);
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Nihao", card->cid.name);
    ret = s_example_write_file(file_nihao, data);
    if (ret != ESP_OK) {
        return;
    }

    //Open file for reading
    ret = s_example_read_file(file_nihao);
    if (ret != ESP_OK) {
        return;
    }

    set_rgb_light(150,30,0,true);

    /*const char *tour = MOUNT_POINT"/tour.wav";
    ret = play_wav(tour);
    if (ret != ESP_OK) {
        return;
    }*/

    

    const char *tour2 = MOUNT_POINT"/promised.wav";
    ret = play_wav(tour2);
    if (ret != ESP_OK) {
        return;
    }

    const char *tour3 = MOUNT_POINT"/dansen.wav";
    ret = play_wav(tour3);
    if (ret != ESP_OK) {
        return;
    }

    const char *tour4 = MOUNT_POINT"/turbo.wav";
    ret = play_wav(tour4);
    if (ret != ESP_OK) {
        return;
    }

    const char *tour5 = MOUNT_POINT"/gold.wav";
    ret = play_wav(tour5);
    if (ret != ESP_OK) {
        return;
    }

    const char *tour6 = MOUNT_POINT"/trip.wav";

    ret = play_wav(tour6);
    if (ret != ESP_OK) {
        return;
    }


    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
    
}

static void *AI_thread(void * arg){

    //return NULL;

    while(true){
    
    //speaker_init(TAG, 44100, I2S_NUM_TX);
    microphone_init(TAG, I2S_SAMPLE_RATE, I2S_NUM_RX);
    
    uint8_t *buffer = (uint8_t *)malloc(I2S_BUFFER_SIZE);
    if (!buffer) {
        ESP_LOGE(TAG, "Memory allocation error");

        //speaker_stop();
        microphone_stop();
        //return NULL;
    }
    else{
        //speaker_play(startData, 78630);
        //speaker_stop();

        //on_dimmable_light();
        //gpio_set_level(BLINK_GPIO, 1);

        microphone_record(buffer, I2S_BUFFER_SIZE);

        //set_dimmable_light_brightness(0, false);
        //gpio_set_level(BLINK_GPIO, 0);

        

        wit_ai_send_audio(TAG, buffer, I2S_BUFFER_SIZE, BLINK_GPIO2, AI_API_URL, AI_API_KEY, AI_API_TYPE);
        //turn_off_normal_light();
        //gpio_set_level(BLINK_GPIO2, 0);

        //speaker_init(TAG, 8000, I2S_NUM_TX);
        //speaker_play(buffer, I2S_BUFFER_SIZE);
        //speaker_stop(); 

        // Release resources
        free(buffer);
        microphone_stop();
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