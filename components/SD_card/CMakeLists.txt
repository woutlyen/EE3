idf_component_register(SRCS SD_card.c
                       INCLUDE_DIRS "include"
                        REQUIRES nvs_flash driver vfs fatfs sdmmc speaker driver)