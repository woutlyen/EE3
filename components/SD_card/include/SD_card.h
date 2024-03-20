
#include "sdmmc_cmd.h"

esp_err_t s_example_write_file(const char *path, char *data);
esp_err_t s_example_read_file(const char *path);
esp_err_t play_wav(const char *path);

void SD_card_init();
void SD_card_deinit();
