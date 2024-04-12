#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "MQTT.h"

#include "motion_sensor.h"
#include "normal_light.h"

#include "mqtt_client.h"
extern esp_mqtt_client_handle_t event_client;

// Motion sensors
#define MOTION_PIN 38
#define MOTION_PIN_2 13
#define GPIO_INPUT_PIN_SEL  ((1ULL<<MOTION_PIN) | (1ULL<<MOTION_PIN_2))

// Function to be called when a motion sensor interrupt occurs
static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            //printf("Rising edge interrupt occurred at GPIO 13!\n");
            printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));

            /*
            if (gpio_get_level(io_num) == 1)
            {
                turn_on_normal_light();
                esp_mqtt_client_publish(event_client, "normal/light/status", "ON", 0, 0, 0);
            }
            else{
                turn_off_normal_light();
                esp_mqtt_client_publish(event_client, "normal/light/status", "OFF", 0, 0, 0);
            }
            */
           
            if (io_num == MOTION_PIN && gpio_get_level(io_num) == 1)
            {
                esp_mqtt_client_publish(event_client, "motion/living/status", "ON", 0, 0, 0);
                if(get_alarm_state()){
                    esp_mqtt_client_publish(event_client, "alarmdecoder/panel", "triggered", 0, 0, 0);
                    set_triggered();
                }
            }
            else if (io_num == MOTION_PIN && gpio_get_level(io_num) == 0){
                esp_mqtt_client_publish(event_client, "motion/living/status", "OFF", 0, 0, 0);
            }
            else if (io_num == MOTION_PIN_2 && gpio_get_level(io_num) == 1){
                esp_mqtt_client_publish(event_client, "motion/kitchen/status", "ON", 0, 0, 0);
            }
            else if (io_num == MOTION_PIN_2 && gpio_get_level(io_num) == 0){
                esp_mqtt_client_publish(event_client, "motion/kitchen/status", "OFF", 0, 0, 0);
            }
            
        }
    }
}

void init_motion_sensor(){

    gpio_config_t io_conf = {};
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 1;
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);

    // Install ISR service
    gpio_install_isr_service(0);

    // Hook ISR handler to GPIO 13 & 8
    gpio_isr_handler_add(MOTION_PIN, gpio_isr_handler, (void*) MOTION_PIN);
    gpio_isr_handler_add(MOTION_PIN_2, gpio_isr_handler, (void*) MOTION_PIN_2);

}