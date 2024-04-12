idf_component_register(SRCS MQTT.c
                       INCLUDE_DIRS "include"
                       REQUIRES mqtt heating_temp_sensor RGB_light dimmable_light normal_light mirf SD)