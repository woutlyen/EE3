#include "stdint.h"
#include "stddef.h"

void speaker_init(const char *T, size_t sample_rate);
void speaker_play(uint8_t *audio_data, size_t audio_size);