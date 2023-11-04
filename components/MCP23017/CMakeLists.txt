idf_component_register(SRCS "mcp23017.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES driver I2C LIS3DH)