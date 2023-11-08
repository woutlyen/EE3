idf_component_register(SRCS WiFi.c WiFi.h
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi nvs_flash)