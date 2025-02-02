idf_component_register(
    SRCS "p1reader.c" 
    INCLUDE_DIRS "."
    REQUIRES  common_buffer esp_driver_uart esp_driver_gpio
    )