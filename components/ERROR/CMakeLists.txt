idf_component_register(SRCS "error.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES Debug SD_CARD GPIO app_comm GPS GPRS)