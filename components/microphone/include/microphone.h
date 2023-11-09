#include "stdint.h"
#include "stddef.h"

void microphone_init(const char *T, size_t sample_rate);
void microphone_record(uint8_t *audio_data, size_t audio_size);