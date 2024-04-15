/**
* Author: Wout Lyen
* Team: HOME3
*/

#ifndef CONFIG_H
#define CONFIG_H

#define I2S_NUM_TX  I2S_NUM_1
#define I2S_NUM_RX  I2S_NUM_0

#define I2S_SAMPLE_RATE (8000)
#define I2S_BUFFER_SIZE (I2S_SAMPLE_RATE * 6)

#define BLINK_GPIO GPIO_NUM_37
#define BLINK_GPIO2 GPIO_NUM_36

#define AI_API_URL "https://api.wit.ai/dictation?v=20231106"
#define AI_API_KEY "V55E3FX2CUEA4XPZXJKULRZ2VLCZD752"
#define AI_API_TYPE "audio/raw;encoding=signed-integer;bits=16;rate=8000;endian=little"

#define ESP_WIFI_SSID      "Galaxy-S20+e7ca"
#define ESP_WIFI_PASS      "jcec9528"


//#define SP_BCK = 11
//#define SP_WS = 10
//#define SP_OUT = 12

#define SP_BCK = 11
#define SP_WS = 10
#define SP_OUT = 12

#define MIC_BCK = 16
#define MIC_WS = 17
#define MIC_IN = 15

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO  GPIO_NUM_37
#define PIN_NUM_MOSI  GPIO_NUM_35
#define PIN_NUM_CLK   GPIO_NUM_36
#define PIN_NUM_CS    GPIO_NUM_20 

#endif