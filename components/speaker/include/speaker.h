#include "stdint.h"
#include "stddef.h"

void speaker_init(const char *T);
void speaker_play(uint8_t *audio_data, size_t audio_size);