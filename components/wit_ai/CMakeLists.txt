idf_component_register(SRCS "wit_ai.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_http_client esp-tls driver normal_light)