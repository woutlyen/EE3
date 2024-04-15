idf_component_register(SRCS SD.c
                       INCLUDE_DIRS "include"
                       REQUIRES vfs fatfs sdmmc MQTT speaker)