/**
* Author: Wout Lyen
* Team: HOME3
*/

#include "stdint.h"
#include "stddef.h"
#include "driver/gpio.h"

void wit_ai_send_audio(const char *T, uint8_t *audio_data, size_t audio_size, gpio_num_t LED, char* API_URL, char* API_KEY, char* API_TYPE);