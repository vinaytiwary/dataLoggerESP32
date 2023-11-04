idf_component_register(SRCS "timer.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES driver esp_timer GPRS Common)