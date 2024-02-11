idf_component_register(SRCS "main.c" 
                    INCLUDE_DIRS "."
                    REQUIRES wit_ai speaker microphone driver pthread esp_wifi nvs_flash protocol_examples_common driver heating_temp_sensor RGB_light dimmable_light normal_light motion_sensor mqtt MQTT)
