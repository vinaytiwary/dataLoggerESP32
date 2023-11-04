idf_component_register(SRCS "gprs.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES esp_timer GPIO I2C PINS config MCP23017 PINS UART Common WEB_COMM Debug Clock)