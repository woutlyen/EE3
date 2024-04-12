#include "mqtt_client.h"
#include "mirf.h"

esp_mqtt_client_handle_t init_mqtt(void);
bool get_music_on_off();
float get_music_volume();
int get_music_next_prev();
bool get_alarm_state();
bool get_triggered();
void set_triggered();
esp_mqtt_client_handle_t get_client();
void clear_music_next_prev();

void init_nrf();
void start_nrf_communication();
void send_nrf_message();
void unlock_door();