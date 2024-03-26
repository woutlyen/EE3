#include <driver/ledc.h>
#include "RGB_light.h"

// Pin numbers for the RGB LED
#define RED_PIN   20
#define GREEN_PIN 19
#define BLUE_PIN  45

// LEDC channel configuration
#define RED_CHANNEL   LEDC_CHANNEL_0
#define GREEN_CHANNEL LEDC_CHANNEL_1
#define BLUE_CHANNEL  LEDC_CHANNEL_2

int rgb[3] = {0, 178, 255};
int rgb_brightness = 255;

void init_rgb_light(){
    // Configure LEDC peripheral
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 1000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);

    // Configure LEDC channels
    ledc_channel_config_t ledc_channel[4] = {
        { .channel = RED_CHANNEL,   .gpio_num = RED_PIN,   .speed_mode = LEDC_LOW_SPEED_MODE },
        { .channel = GREEN_CHANNEL, .gpio_num = GREEN_PIN, .speed_mode = LEDC_LOW_SPEED_MODE },
        { .channel = BLUE_CHANNEL,  .gpio_num = BLUE_PIN,  .speed_mode = LEDC_LOW_SPEED_MODE }
    };
    for (int i = 0; i < 3; i++) {
        ledc_channel_config(&ledc_channel[i]);
    }
}


void set_rgb_light(int red, int green, int blue, bool update) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, RED_CHANNEL, (red*rgb_brightness/255)*4);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, GREEN_CHANNEL, (green*rgb_brightness/255)*4);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BLUE_CHANNEL, (blue*rgb_brightness/255)*4);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, RED_CHANNEL);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, GREEN_CHANNEL);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BLUE_CHANNEL);

    if (update)
    {
        rgb[0] = red;
        rgb[1] = green;
        rgb[2] = blue;
    }
    
}

void on_rgb_light() {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, RED_CHANNEL, (rgb[0]*rgb_brightness/255)*4);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, GREEN_CHANNEL, (rgb[1]*rgb_brightness/255)*4);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BLUE_CHANNEL, (rgb[2]*rgb_brightness/255)*4);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, RED_CHANNEL);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, GREEN_CHANNEL);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BLUE_CHANNEL);
}

void set_rgb_light_brightness(int brightness){
    rgb_brightness = brightness;
    on_rgb_light();
}

int get_red(){
    return rgb[0];
}

int get_green(){
    return rgb[1];
}

int get_blue(){
    return rgb[2];
}

int get_rgb_light_brightness(){
    return rgb_brightness;
}