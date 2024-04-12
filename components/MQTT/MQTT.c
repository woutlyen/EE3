#include "esp_log.h"

#include "heating_temp_sensor.h"
#include "RGB_light.h"
#include "dimmable_light.h"
#include "normal_light.h"
#include "MQTT.h"
#include "SD.h"

extern esp_mqtt_client_handle_t event_client;

static const char *TAG = "MQTT";

int music_volume = 50;
bool music_on_off = false;
int music_next_prev = 0;

bool alarm_on = false;
bool triggered = false;

int hvac_temperature = 21;
int current_temperature = 20;
int modus = 0;

NRF24_t dev;
uint8_t message[5];

bool door_unlock = false; 

void unlock_door(){
    door_unlock = true;
}

esp_mqtt_client_handle_t get_client(){
    return event_client;
}

bool get_triggered(){
    return triggered;
}

void set_triggered(){
    triggered = true;
}

bool get_alarm_state(){
    return alarm_on;
}

void publish_dimmable_light_brightness(){
    char status[4];
    sprintf(status,"%d", get_dimmable_light_brightness());
    esp_mqtt_client_publish(event_client, "dim/brightness/status", status, 0, 0, 0);
}

void publish_rgb_light(){
    char status[11];
    sprintf(status,"%d,%d,%d", get_red(), get_green(), get_blue());
    esp_mqtt_client_publish(event_client, "rgb/rgb/status", status, 0, 0, 0);
}

void publish_rgb_light_brightness(){
    char status[4];
    sprintf(status,"%d", get_rgb_light_brightness());
    esp_mqtt_client_publish(event_client, "rgb/brightness/status", status, 0, 0, 0);
}

void publish_volume(){
    char status[4];
    sprintf(status,"%d", music_volume);
    esp_mqtt_client_publish(event_client, "music/volume/status", status, 0, 0, 0);
}

bool get_music_on_off(){
    return music_on_off;
}

float get_music_volume(){
    return (float) music_volume/255;
} 

int get_music_next_prev(){
    return music_next_prev;
}

void clear_music_next_prev(){
    music_next_prev = 0;
}


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void mqtt_connected(){
    int msg_id;
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

    msg_id = esp_mqtt_client_subscribe(event_client, "rgb/rgb/set", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "rgb/light/switch", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "rgb/brightness/set", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);


    msg_id = esp_mqtt_client_subscribe(event_client, "dim/light/switch", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "dim/brightness/set", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);


    msg_id = esp_mqtt_client_subscribe(event_client, "music/switch", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "music/volume/set", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "music/previous", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "music/pause", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "music/next", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    
    msg_id = esp_mqtt_client_subscribe(event_client, "alarmdecoder/panel/set", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);


    //lights node
    msg_id = esp_mqtt_client_subscribe(event_client, "rgb2/rgb/set", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "rgb2/light/switch", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "rgb2/brightness/set", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(event_client, "dim2/light/switch", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "dim2/brightness/set", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(event_client, "normal/light/switch", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(event_client, "hvac/mode/set", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(event_client, "hvac/temp/set", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
}

void mqtt_subscribed(){

    
    on_rgb_light();
    esp_mqtt_client_publish(event_client, "rgb/light/status", "ON", 0, 0, 0);
    publish_rgb_light();
    publish_rgb_light_brightness();

    on_dimmable_light();
    esp_mqtt_client_publish(event_client, "dim/light/status", "ON", 0, 0, 0);
    publish_dimmable_light_brightness();

    //turn_on_normal_light();
    //esp_mqtt_client_publish(event_client, "normal/light/status", "ON", 0, 0, 0);

    //esp_mqtt_client_publish(event_client, "hvac/mode/status", "heat", 0, 0, 0);

    //esp_mqtt_client_publish(event_client, "hvac/temp/status", "21.0", 0, 0, 0);
    //esp_mqtt_client_publish(event_client, "hvac/temp/current", "20.0", 0, 0, 0);

    esp_mqtt_client_publish(event_client, "motion/downstairs/status", "OFF", 0, 0, 0);
    esp_mqtt_client_publish(event_client, "motion/upstairs/status", "OFF", 0, 0, 0);

    esp_mqtt_client_publish(event_client, "music/status", "OFF", 0, 0, 0);
    esp_mqtt_client_publish(event_client, "music/volume/status", "50", 0, 0, 0);
    
}


/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    //event_client = client;
    //int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        mqtt_connected();
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        mqtt_subscribed();
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");

        char mqtt_topic[100];
        memset(mqtt_topic, 0, sizeof mqtt_topic);

        //printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        sprintf(mqtt_topic, "%.*s", event->topic_len, event->topic);
        printf("TOPIC = %s\n", mqtt_topic);

        if (strcmp(mqtt_topic, "rgb/light/switch") == 0)
        {
            char rgb_on_off[10];
            sprintf(rgb_on_off, "%.*s", event->data_len, event->data);

            if (strcmp(rgb_on_off, "ON") == 0)
            {
                on_rgb_light();
                esp_mqtt_client_publish(client, "rgb/light/status", rgb_on_off, 0, 0, 0);
            }
            else if(strcmp(rgb_on_off, "OFF") == 0)
            {
                set_rgb_light(0, 0, 0, false);
                esp_mqtt_client_publish(client, "rgb/light/status", rgb_on_off, 0, 0, 0);
            }
            
        }
        
        else if (strcmp(mqtt_topic, "rgb/rgb/set") == 0)
        {
            char temp[11];
            int rgb[3] = {0, 0, 0};
            sprintf(temp, "%.*s\r\n", event->data_len, event->data);

            // Tokenize the string based on the comma
            char* token = strtok(temp, ",");
        
            // Convert each token to an integer and store it in the array
            for (int i = 0; i < 3 && token != NULL; ++i) {
                rgb[i] = atoi(token);
                token = strtok(NULL, ",");
            }

            //printf("RED  = %d - GREEN = %d - BLUE = %d\n", rgb[0], rgb[1], rgb[2]);
            set_rgb_light(rgb[0], rgb[1], rgb[2], true);

            publish_rgb_light();
        }

        else if (strcmp(mqtt_topic, "rgb/brightness/set") == 0)
        {
            char temp[4];
            sprintf(temp, "%.*s\r\n", event->data_len, event->data);
            set_rgb_light_brightness(atoi(temp));
        
            publish_rgb_light_brightness();
        }

        else if (strcmp(mqtt_topic, "dim/light/switch") == 0)
        {
            char rgb_on_off[10];
            sprintf(rgb_on_off, "%.*s", event->data_len, event->data);

            if (strcmp(rgb_on_off, "ON") == 0)
            {
                on_dimmable_light();
                esp_mqtt_client_publish(client, "dim/light/status", rgb_on_off, 0, 0, 0);
            }
            else if(strcmp(rgb_on_off, "OFF") == 0)
            {
                set_dimmable_light_brightness(0, false);
                esp_mqtt_client_publish(client, "dim/light/status", rgb_on_off, 0, 0, 0);
            }
            
        }

        else if (strcmp(mqtt_topic, "dim/brightness/set") == 0)
        {
            char temp[4];
            sprintf(temp, "%.*s\r\n", event->data_len, event->data);
            set_dimmable_light_brightness(atoi(temp), true);
        
            publish_dimmable_light_brightness();
        }

        else if (strcmp(mqtt_topic, "music/switch") == 0)
        {
            char on_off[10];
            sprintf(on_off, "%.*s", event->data_len, event->data);

            if (strcmp(on_off, "ON") == 0)
            {
                music_on_off = true;
                esp_mqtt_client_publish(client, "music/status", on_off, 0, 0, 0);
            }
            else if(strcmp(on_off, "OFF") == 0)
            {
                music_on_off = false;
                esp_mqtt_client_publish(client, "music/status", on_off, 0, 0, 0);
            }
            
        }

        else if (strcmp(mqtt_topic, "music/volume/set") == 0)
        {
            char temp[4];
            sprintf(temp, "%.*s\r\n", event->data_len, event->data);
            music_volume = (atoi(temp));
        
            publish_volume();
        }

        else if (strcmp(mqtt_topic, "music/previous") == 0)
        {
         music_next_prev = 1;
        }

        else if (strcmp(mqtt_topic, "music/next") == 0)
        {
         music_next_prev = 2;
        }


        else if (strcmp(mqtt_topic, "music/pause") == 0)
        {
         music_on_off = !music_on_off;

            if (music_on_off)
            {
                esp_mqtt_client_publish(client, "music/status", "ON", 0, 0, 0);
            }
            else
            {
                esp_mqtt_client_publish(client, "music/status", "OFF", 0, 0, 0);
            }
        }

        /*
        else if (strcmp(mqtt_topic, "hvac/mode/set") == 0)
        {
            char mode[10];
            sprintf(mode, "%.*s", event->data_len, event->data);

            set_hvac_mode(!strcmp(mode, "off"));

        }

        else if (strcmp(mqtt_topic, "hvac/temp/set") == 0)
        {
            char* temp = (char*)malloc(sizeof(char)*100);
            
            sprintf(temp, "%.*s\r\n", event->data_len, event->data);
            printf(temp);
            set_hvac_temp(atof(temp));
            free(temp);

        }*/

        else if (strcmp(mqtt_topic, "alarmdecoder/panel/set") == 0)
        {
            char mode[10];
            sprintf(mode, "%.*s", event->data_len, event->data);

            if (strcmp(mode, "ARM_AWAY") == 0)
            {
                alarm_on = true;
                esp_mqtt_client_publish(client, "alarmdecoder/panel", "armed_away", 0, 0, 0);
            }
            else if(strcmp(mode, "DISARM") == 0)
            {
                alarm_on = false;
                triggered = false;
                esp_mqtt_client_publish(client, "alarmdecoder/panel", "disarmed", 0, 0, 0);
            }

        }


        else if (strcmp(mqtt_topic, "normal/light/switch") == 0)
        {
            char on_off[10];
            sprintf(on_off, "%.*s", event->data_len, event->data);

            if (strcmp(on_off, "ON") == 0)
            {
                memset(message, 0, sizeof(message));
                message[0] = 1;
                message[1] = 1;
                send_nrf_message();
            }
            else if(strcmp(on_off, "OFF") == 0)
            {
                memset(message, 0, sizeof(message));
                message[0] = 1;
                message[1] = 2;
                send_nrf_message();
            }
        }

        else if (strcmp(mqtt_topic, "dim2/light/switch") == 0)
        {
            char on_off[10];
            sprintf(on_off, "%.*s", event->data_len, event->data);

            if (strcmp(on_off, "ON") == 0)
            {
                memset(message, 0, sizeof(message));
                message[0] = 2;
                message[1] = 1;
                send_nrf_message();
            }
            else if(strcmp(on_off, "OFF") == 0)
            {
                memset(message, 0, sizeof(message));
                message[0] = 2;
                message[1] = 2;
                send_nrf_message();
            }
        }

        else if (strcmp(mqtt_topic, "rgb2/light/switch") == 0)
        {
            char on_off[10];
            sprintf(on_off, "%.*s", event->data_len, event->data);

            if (strcmp(on_off, "ON") == 0)
            {
                memset(message, 0, sizeof(message));
                message[0] = 3;
                message[1] = 1;
                send_nrf_message();
            }
            else if(strcmp(on_off, "OFF") == 0)
            {
                memset(message, 0, sizeof(message));
                message[0] = 3;
                message[1] = 2;
                send_nrf_message();
            }
        }

        else if (strcmp(mqtt_topic, "dim2/brightness/set") == 0)
        {
            char temp[4];
            sprintf(temp, "%.*s\r\n", event->data_len, event->data);
            memset(message, 0, sizeof(message));
            message[0] = 2;
            message[1] = 3;
            message[2] = atoi(temp);
            send_nrf_message();
        }

        else if (strcmp(mqtt_topic, "rgb2/brightness/set") == 0)
        {
            char temp[4];
            sprintf(temp, "%.*s\r\n", event->data_len, event->data);
            memset(message, 0, sizeof(message));
            message[0] = 3;
            message[1] = 3;
            message[2] = atoi(temp);
            send_nrf_message();
        }

        else if (strcmp(mqtt_topic, "rgb2/rgb/set") == 0)
        {
            char temp[11];
            int rgb[3] = {0, 0, 0};
            sprintf(temp, "%.*s\r\n", event->data_len, event->data);

            // Tokenize the string based on the comma
            char* token = strtok(temp, ",");
        
            // Convert each token to an integer and store it in the array
            for (int i = 0; i < 3 && token != NULL; ++i) {
                rgb[i] = atoi(token);
                token = strtok(NULL, ",");
            }
            
            memset(message, 0, sizeof(message));
            message[0] = 3;
            message[1] = 4;
            message[2] = rgb[0];
            message[3] = rgb[1];
            message[4] = rgb[2];
            send_nrf_message();
        }

        else if (strcmp(mqtt_topic, "hvac/mode/set") == 0)
        {
            char mode[10];
            sprintf(mode, "%.*s", event->data_len, event->data);

            memset(message, 0, sizeof(message));
            message[0] = 4;

            if (!strcmp(mode, "off")){
                message[1] = 2;
            }
            else if (!strcmp(mode, "cool") || !strcmp(mode, "heat") ){
                message[1] = 1;
            }

            send_nrf_message();

        }

        else if (strcmp(mqtt_topic, "hvac/temp/set") == 0)
        {
            char* temp = (char*)malloc(sizeof(char)*100);
            
            sprintf(temp, "%.*s\r\n", event->data_len, event->data);
            printf(temp);

            memset(message, 0, sizeof(message));
            message[0] = 4;
            message[1] = 3;
            message[2] = atoi(temp);
            send_nrf_message();
            free(temp);

        }

        break;


    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

esp_mqtt_client_handle_t init_mqtt(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    return(client);
}

void init_nrf(){

        ESP_LOGI("NRF", "Start");
        Nrf24_init(&dev);
        uint8_t payload = 5;
        uint8_t channel = CONFIG_RADIO_CHANNEL;
        Nrf24_config(&dev, channel, payload);

        //Set own address using 5 characters
        esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"FGHIJ");
        if (ret != ESP_OK) {
            ESP_LOGE("NRF", "nrf24l01 not installed");
            while(1) { vTaskDelay(1); }
        }

        //Set the receiver address using 5 characters
        ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
        if (ret != ESP_OK) {
            ESP_LOGE("NRF", "nrf24l01 not installed");
            while(1) { vTaskDelay(1); }
        }

        ESP_LOGW("NRF", "Set RF Data Ratio to 1MBps");
        Nrf24_SetSpeedDataRates(&dev, 0);
        //Nrf24_SetSpeedDataRates(&dev, 1);

        ESP_LOGW("NRF", "CONFIG_RETRANSMIT_DELAY=%d", CONFIG_RETRANSMIT_DELAY);
        Nrf24_setRetransmitDelay(&dev, CONFIG_RETRANSMIT_DELAY);

        //Print settings
        Nrf24_printDetails(&dev);
    }

void start_nrf_communication(){
        ESP_LOGI("NRF", "Listening...");

        uint8_t buf[5];

        // Clear RX FiFo
        while(1) {
            if (Nrf24_dataReady(&dev) == false) break;
            Nrf24_getData(&dev, buf);
        }

        while(1) {
            //When the program is received, the received data is output from the serial port
            if (Nrf24_dataReady(&dev)) {
                Nrf24_getData(&dev, buf);
                ESP_LOGI("NRF", "Got data:%s", buf);

                /*Voice Assistant*/
                if(buf[0] == 8 && buf[1] == 1){ //Start voice assistant
                    set_voice_assistant_activated();
                }

                /*Normal LED*/
                if(buf[0] == 1 && buf[1] == 17){ //Normal LED turned on
                    esp_mqtt_client_publish(event_client, "normal/light/status", "ON", 0, 0, 0);
                }
                else if(buf[0] == 1 && buf[1] == 18){ //Normal LED turned off
                    esp_mqtt_client_publish(event_client, "normal/light/status", "OFF", 0, 0, 0);
                }


                /*Dimmable LED*/
                if(buf[0] == 2 && buf[1] == 17){ //Dimmable LED turned on
                    esp_mqtt_client_publish(event_client, "dim2/light/status", "ON", 0, 0, 0);
                }
                else if(buf[0] == 2 && buf[1] == 18){ //Dimmable LED turned off
                    esp_mqtt_client_publish(event_client, "dim2/light/status", "OFF", 0, 0, 0);
                }
                else if(buf[0] == 2 && buf[1] == 19){ //Dimmable LED new brightness
                    char status[4];
                    sprintf(status,"%d", buf[2]);
                    esp_mqtt_client_publish(event_client, "dim2/brightness/status", status, 0, 0, 0);
                }


                /*RGB LED*/
                if(buf[0] == 3 && buf[1] == 17){ //RGB LED turned on
                    esp_mqtt_client_publish(event_client, "rgb2/light/status", "ON", 0, 0, 0);
                }
                else if(buf[0] == 3 && buf[1] == 18){ //RGB LED turned off
                    esp_mqtt_client_publish(event_client, "rgb2/light/status", "ON", 0, 0, 0);
                }
                else if(buf[0] == 3 && buf[1] == 19){ //RGB LED new brightness
                    char status[4];
                    sprintf(status,"%d", buf[2]);
                    esp_mqtt_client_publish(event_client, "rgb2/brightness/status", status, 0, 0, 0);
                }
                else if(buf[0] == 3 && buf[1] == 20){ //RGB LED new color
                    char status[12];
                    sprintf(status,"%d,%d,%d", buf[2], buf[3], buf[4]);
                    esp_mqtt_client_publish(event_client, "rgb2/rgb/status", status, 0, 0, 0);
                }


                /*Heating System*/
                if(buf[0] == 4 && buf[1] == 17){ //Heating turned on
                    modus = 1;
                    if (hvac_temperature >= current_temperature)
                    {
                        esp_mqtt_client_publish(event_client, "hvac/mode/status", "heat", 0, 0, 0);
                    }
                    else if (hvac_temperature < current_temperature)
                    {
                        esp_mqtt_client_publish(event_client, "hvac/mode/status", "cool", 0, 0, 0);
                    }
                }
                else if(buf[0] == 4 && buf[1] == 18){ //Heating turned off
                    modus = 0;
                    esp_mqtt_client_publish(event_client, "hvac/mode/status", "off", 0, 0, 0);
                }
                else if(buf[0] == 4 && buf[1] == 19){ //Heating new temperature
                    hvac_temperature = buf[2];
                    char status[4];
                    sprintf(status,"%d", buf[2]);
                    esp_mqtt_client_publish(event_client, "hvac/temp/status", status, 0, 0, 0);
                }


                /*Motion Sensor*/
                if(buf[0] == 5 && buf[1] == 1){ //Motion Sensor turned on
                    esp_mqtt_client_publish(event_client, "motion/kitchen/status", "ON", 0, 0, 0);
                }
                else if(buf[0] == 5 && buf[1] == 2){ //Motion Sensor turned off
                    esp_mqtt_client_publish(event_client, "motion/kitchen/status", "OFF", 0, 0, 0);
                }


                /*Temperature Sensor*/
                if(buf[0] == 7 && buf[1] == 1){ //Temperature measurement
                    if(buf[2] >= 10 && buf[2] <= 35){
                        current_temperature = buf[2];
                        char status[3];
                        sprintf(status,"%d", buf[2]);
                        esp_mqtt_client_publish(event_client, "hvac/temp/current", status, 0, 0, 0);
                    }
                    else if(buf[2] < 10){
                        current_temperature = 10;
                        char status[3];
                        sprintf(status,"%d", 10);
                        esp_mqtt_client_publish(event_client, "hvac/temp/current", status, 0, 0, 0);
                    }
                    else if(buf[2] > 35){
                        current_temperature = 35;
                        char status[3];
                        sprintf(status,"%d", 35);
                        esp_mqtt_client_publish(event_client, "hvac/temp/current", status, 0, 0, 0);
                    }

                    if (modus != 0)
                    {
                        if (hvac_temperature >= current_temperature)
                        {
                            esp_mqtt_client_publish(event_client, "hvac/mode/status", "heat", 0, 0, 0);
                        }
                        else if (hvac_temperature < current_temperature)
                        {
                            esp_mqtt_client_publish(event_client, "hvac/mode/status", "cool", 0, 0, 0);
                        }
                    }
                }
            }
            else if (door_unlock){
                uint8_t mes[5];
                memset(mes, 0, sizeof(mes));

                mes[0] = 8;
                mes[1] = 2;

                Nrf24_send(&dev, mes);

                ESP_LOGI("NRF", "Wait for sending.....");
                if (Nrf24_isSend(&dev, 10000)) {
                    ESP_LOGI("NRF","Send success:%s", mes);
                } else {
                    ESP_LOGW("NRF","Send fail:");
                }
            }
            door_unlock = false;
            vTaskDelay(1);
        }

}

void send_nrf_message(){
    Nrf24_send(&dev, message);
    ESP_LOGI("NRF", "Wait for sending.....");
    if (Nrf24_isSend(&dev, 10000)) {
        ESP_LOGI("NRF","Send success:%s", message);
    } else {
        ESP_LOGW("NRF","Send fail:");
    }
    vTaskDelay(1);
}