idf_component_register(SRCS "accelerometer.cpp"
                    INCLUDE_DIRS "."
            REQUIRES UART GPRS  GPS WEB_COMM Debug ERROR MCP23017)