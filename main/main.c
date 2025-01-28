#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hal/uart_types.h"
#include "hal/gpio_types.h"

#include "secrets.h"

#include "ringbuf.h"
#include "p1reader.h"
#include "server.h"
#include "wifi.h"

static const char *TAG = "p1-esp32";

void app_main(void) {
    wifi_init(WIFI_ACCESPOINT, WIFI_PASSWORD);

    ringbuf_t rb1 = ringbuf_new(2048);
    p1_init(UART_NUM_0, GPIO_NUM_0, rb1);

    server_start(23, rb1);

}
