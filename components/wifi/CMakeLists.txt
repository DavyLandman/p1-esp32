idf_component_register(
    SRCS "wifi.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES "nvs_flash"
    REQUIRES "esp_wifi" "esp_netif"
    )