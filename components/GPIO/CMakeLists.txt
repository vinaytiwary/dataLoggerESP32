idf_component_register(SRCS "GPIO.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES driver I2C MCP23017 PINS Debug app_comm SD_CARD Common)