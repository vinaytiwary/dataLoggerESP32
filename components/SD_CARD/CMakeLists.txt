idf_component_register(SRCS "Handle_sdcard.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES fatfs UART Common GPRS GPS WEB_COMM Debug Clock EEPROM RTC ADC ERROR)