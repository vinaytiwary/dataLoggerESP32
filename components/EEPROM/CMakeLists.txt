idf_component_register(SRCS "EEPROM.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES Debug RTC nvs_flash Common UART SD_CARD ERROR)