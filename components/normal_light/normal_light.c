#include <driver/gpio.h>
#include "normal_light.h"

// Pin numbers for the LED
#define NORMAL_PIN 36

void init_normal_light(){
    
    gpio_reset_pin(NORMAL_PIN);
    gpio_set_direction(NORMAL_PIN, GPIO_MODE_OUTPUT);

}

void turn_on_normal_light(){
    gpio_set_level(NORMAL_PIN, 1);
}

void turn_off_normal_light(){
    gpio_set_level(NORMAL_PIN, 0);
}
