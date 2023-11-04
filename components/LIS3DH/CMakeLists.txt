idf_component_register(SRCS "lis3dh.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES driver I2C SD_CARD Debug esp_timer)