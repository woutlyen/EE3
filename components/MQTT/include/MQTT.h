#include "mqtt_client.h"

esp_mqtt_client_handle_t init_mqtt(void);
bool get_music_on_off();
float get_music_volume();
int get_music_next_prev();
bool get_alarm_state();
void clear_music_next_prev();