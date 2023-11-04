idf_component_register(SRCS "gps.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES driver I2C esp_timer GPIO PINS config UART Common GPRS WEB_COMM Debug Clock SD_CARD ERROR)