#include "stdint.h"
#include "stddef.h"
#include "driver/i2s.h"

void speaker_init(const char *T, size_t sample_rate, i2s_port_t PORT);
void speaker_play(uint8_t *audio_data, size_t audio_size);