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

#endif