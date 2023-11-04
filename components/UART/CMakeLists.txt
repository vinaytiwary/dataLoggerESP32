idf_component_register(SRCS "UART.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES driver GPRS app_comm Common)