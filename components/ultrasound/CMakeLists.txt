idf_component_register(SRCS "ultrasound.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES driver
                       PRIV_REQUIRES log esp_timer)
