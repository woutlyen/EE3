#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"

#include "speaker.h"
#include "MQTT.h"

#define I2S_NUM_TX  I2S_NUM_1
#define MOUNT_POINT "/sdcard"
#define AUDIO_BUFFER 1024

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO  GPIO_NUM_7
#define PIN_NUM_MOSI  GPIO_NUM_5
#define PIN_NUM_CLK   GPIO_NUM_6
#define PIN_NUM_CS    GPIO_NUM_4 

static const char *TAG = "SD";

esp_err_t play_wav(const char *path)
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
  uint8_t *emptybuf = calloc(AUDIO_BUFFER, sizeof(int8_t));
  uint16_t *inbuf = calloc(AUDIO_BUFFER/2, sizeof(int16_t));
  size_t bytes_read = 0;

  bytes_read = fread(inbuf, sizeof(int16_t), AUDIO_BUFFER/2, fh);

  speaker_init(TAG, 48000, I2S_NUM_TX);

  int16_t sample = 0;
  float factor = 0.5;

  while (bytes_read > 0 && get_music_next_prev() == 0)
  {
    //ESP_LOGI(TAG, "Music %d, %u", get_music_on_off(), bytes_read);

    while(get_triggered()){
        FILE *alarm = fopen("/sdcard/alarm.wav", "rb");
        if (alarm == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file");
            return ESP_ERR_INVALID_ARG;
        }
        // skip the header...
        fseek(alarm, 44, SEEK_SET);

        // create a writer buffer
        uint8_t *alarm_buf = calloc(AUDIO_BUFFER, sizeof(int8_t));
        size_t read = 0;
        read = fread(alarm_buf, sizeof(int8_t), AUDIO_BUFFER, alarm);

        while (read > 0 && get_alarm_state())
        {
            speaker_play(alarm_buf, AUDIO_BUFFER);
            read = fread(alarm_buf, sizeof(int8_t), AUDIO_BUFFER, alarm);
        }
    free(alarm_buf);
    fclose(alarm);
    }

    if (get_music_on_off() == true){
        factor = get_music_volume();
        for (size_t i = 0; i < AUDIO_BUFFER/2; i++)
        {
            sample = inbuf[i];
            sample = (sample*factor);
            buf[2*i] = sample & 0xFF;
            buf[2*i+1] = (sample >> 8);

        }

        speaker_play(buf, AUDIO_BUFFER);
        // write the buffer to the i2s
        //i2s_channel_write(tx_handle, buf, bytes_read * sizeof(int16_t), &bytes_written, portMAX_DELAY);
        bytes_read = fread(inbuf, sizeof(int16_t), AUDIO_BUFFER/2, fh);
        ESP_LOGV(TAG, "Bytes read: %d", bytes_read);
    }
    else{
        speaker_play(emptybuf, AUDIO_BUFFER);
    }
    
  }

  //i2s_channel_disable(tx_handle);
  speaker_stop();
  free(buf);
  free(inbuf);
  fclose(fh);
  return ESP_OK;
}


void init_SD(void){
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


    const char *songs[] = {"promised", "tour", "dansen", "turbo", "gold", "trip"};

    int MAX_PATH_LENGTH = 100;
    int ARRAY_SIZE = 6;
    int CURRENT_LOC = 0;

    char song[MAX_PATH_LENGTH];

    while(true){

        snprintf(song, MAX_PATH_LENGTH, "%s/%s.wav", MOUNT_POINT, songs[CURRENT_LOC]);

        ESP_LOGI(TAG, "%d %s", CURRENT_LOC, song);

        ret = play_wav(song);
        if (ret != ESP_OK) {
            return;
        }

        if(get_music_next_prev() == 2){
            CURRENT_LOC++;
            CURRENT_LOC = CURRENT_LOC % (ARRAY_SIZE);
        }
        else if (get_music_next_prev() == 1){
            CURRENT_LOC--;
            CURRENT_LOC = (CURRENT_LOC+ARRAY_SIZE) % (ARRAY_SIZE);
        }
        else{
            CURRENT_LOC++;
            CURRENT_LOC = CURRENT_LOC % (ARRAY_SIZE);
        }
        clear_music_next_prev();
    }

    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
}