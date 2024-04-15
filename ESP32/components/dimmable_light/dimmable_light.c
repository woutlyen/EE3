/**
* Author: Wout Lyen
* Team: HOME3
*/

#include <driver/ledc.h>
#include "dimmable_light.h"

// Pin number for the dimmable LED
#define WHITE_PIN  37

// LEDC channel configuration
#define WHITE_CHANNEL  LEDC_CHANNEL_3

int brightness = 200;

void init_dimmable_light(){
    
    // Configure LEDC peripheral
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 1000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);

    // Configure LEDC channels
    ledc_channel_config_t ledc_channel[1] = {
        { .channel = WHITE_CHANNEL,  .gpio_num = WHITE_PIN,  .speed_mode = LEDC_LOW_SPEED_MODE }
    };
    for (int i = 0; i < 1; i++) {
        ledc_channel_config(&ledc_channel[i]);
    }

}

void on_dimmable_light(){
    ledc_set_duty(LEDC_LOW_SPEED_MODE, WHITE_CHANNEL, brightness*4);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, WHITE_CHANNEL);
}

void set_dimmable_light_brightness(int b, bool update){
    ledc_set_duty(LEDC_LOW_SPEED_MODE, WHITE_CHANNEL, b*4);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, WHITE_CHANNEL);

    if (update)
    {
        brightness = b;
    }
    
}

int get_dimmable_light_brightness(){
    return brightness;
}