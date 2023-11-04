idf_component_register(SRCS "app_comm.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES config UART GPRS Common GPS WEB_COMM Debug ERROR)