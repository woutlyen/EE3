idf_component_register(SRCS motion_sensor.c
                       INCLUDE_DIRS "include"
                       REQUIRES mqtt driver freertos normal_light MQTT)