#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include <math.h>
#include "WavData.h"

static const i2s_port_t i2s_num = I2S_NUM_0;  // i2s port number
unsigned char* TheData;
uint32_t DataIdx=0;                           // index offset into "TheData" for current  data t send to I2S

struct WavHeader_Struct
    {
      //   RIFF Section    
      char RIFFSectionID[4];      // Letters "RIFF"
      uint32_t Size;              // Size of entire file less 8
      char RiffFormat[4];         // Letters "WAVE"
      
      //   Format Section    
      char FormatSectionID[4];    // letters "fmt"
      uint32_t FormatSize;        // Size of format section less 8
      uint16_t FormatID;          // 1=uncompressed PCM
      uint16_t NumChannels;       // 1=mono,2=stereo
      uint32_t SampleRate;        // 44100, 16000, 8000 etc.
      uint32_t ByteRate;          // =SampleRate * Channels * (BitsPerSample/8)
      uint16_t BlockAlign;        // =Channels * (BitsPerSample/8)
      uint16_t BitsPerSample;     // 8,16,24 or 32
    
      // Data Section
      char DataSectionID[4];      // The letters "data"
      uint32_t DataSize;          // Size of the data that follows
    }WavHeader;

// Fill the output buffer and write to I2S DMA
static void write_buffer()
{
    size_t BytesWritten;  

    // Write with max delay. We want to push buffers as fast as we
    // can into DMA memory. If DMA memory isn't transmitted yet this
    // will yield the task until the interrupt fires when DMA buffer has 
    // space again. If we aren't keeping up with the real-time deadline,
    // audio will glitch and the task will completely consume the CPU,
    // not allowing any task switching interrupts to be processed.
    i2s_write(i2s_num, WavData+44+DataIdx,4,&BytesWritten, portMAX_DELAY);
    DataIdx+=4;  
    if(DataIdx>=165612-44)               // If we gone past end of data reset back to beginning
    DataIdx=0;                                 

    // You could put a taskYIELD() here to ensure other tasks always have a chance to run.
    // taskYIELD();
}

static void audio_task(void *userData)
{
    while(1) {
        write_buffer();
    }
}

void app_main(void)
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_STAND_MSB,
        .dma_buf_count = 8,
        .dma_buf_len = 1023,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = 11,
        .ws_io_num = 10,
        .data_out_num = 12,
        .data_in_num = -1                                                       //Not used
    };

    //memcpy(&WavHeader,&WavData,44);
    //if(ValidWavData(&WavHeader))
    //{
      i2s_driver_install(i2s_num, &i2s_config, 0, NULL);        // ESP32 will allocated resources to run I2S
      i2s_set_pin(i2s_num, &pin_config);                        // Tell it the pins you will be using
      i2s_set_sample_rates(i2s_num, 44100);      //set sample rate 
      TheData=WavData;                                          // set to start of data  
      TheData+=44;                       
    //}    

    // Highest possible priority for realtime audio task
    xTaskCreate(audio_task, "audio", 1024, NULL, configMAX_PRIORITIES - 1, NULL);
}