/**
* Author: Wout Lyen
* Team: HOME3
*/

#include "stdint.h"
#include "stddef.h"
#include "driver/i2s.h"

void microphone_init(const char *T, size_t sample_rate, i2s_port_t PORT);
void microphone_record(uint8_t *audio_data, size_t audio_size);
void microphone_stop(void);