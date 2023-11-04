idf_component_register(SRCS "clock.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES GPRS config UART Common WEB_COMM Debug RTC GPS EEPROM SD_CARD ERROR)