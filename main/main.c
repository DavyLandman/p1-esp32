#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hal/uart_types.h"
#include "hal/gpio_types.h"

#include "secrets.h"

#include "p1reader.h"
#include "server.h"
#include "wifi.h"
#include "common_buffer.h"

static const char *TAG = "p1-esp32";

#ifdef CONFIG_IDF_TARGET_ESP32S2 
#define UART0_RX_PIN GPIO_NUM_40
#define UART1_RX_PIN GPIO_NUM_38
#elif CONFIG_IDF_TARGET_ESP32C3
#define UART0_RX_PIN GPIO_NUM_3
#define UART1_RX_PIN GPIO_NUM_4
#else
#error "missing config for uarts pins"
#endif


void app_main(void) {
    wifi_start(WIFI_ACCESPOINT, WIFI_PASSWORD);

    common_buffer_t* buf1 = allocate(2048);
    common_buffer_t* buf2 = allocate(2048);

    p1_start(UART_NUM_0, UART0_RX_PIN, buf1);
    p1_start(UART_NUM_1, UART1_RX_PIN, buf2);

    server_start(18, buf1);
    server_start(23, buf2);

}
