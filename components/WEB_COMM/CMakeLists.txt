idf_component_register(SRCS "web_comm.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES esp_timer GPIO PINS config UART GPRS Common app_comm GPS Debug)