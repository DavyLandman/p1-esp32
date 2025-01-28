idf_component_register(
    SRCS "p1reader.c" 
    INCLUDE_DIRS "."
    REQUIRES  esp_driver_uart esp_driver_gpio ringbuf
    )