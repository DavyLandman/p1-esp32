#include "p1reader.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "p1-reader";

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


struct config {
    ringbuf_t target;
    uart_port_t port;
};

#define ROUGH_P1_SIZE 1024

static void read_serial_task(struct config const * ctx) {
    char *buffer = malloc(ROUGH_P1_SIZE + 1);
    while(true) {
        int read = uart_read_bytes(ctx->port, buffer, ROUGH_P1_SIZE, pdMS_TO_TICKS(50));
        if (read > 0) {
            buffer[read] = '\0';
            ringbuf_memcpy_into(ctx->target, buffer, read);
        }
    }
}


#if CONFIG_UART_ISR_IN_IRAM
    #define __INTR_FLAGS ESP_INTR_FLAG_IRAM
#else
    #define __INTR_FLAGS 0
#endif

static void setup_uart(const uart_port_t uart_num, gpio_num_t rx_pin) {
    ESP_LOGI(TAG, "Starting serial setup");
    // Configure UART parameters
    ESP_LOGI(TAG, "uart param");

    uart_config_t p1_serial_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    ESP_ERROR_CHECK(uart_param_config(uart_num, &p1_serial_config));
    ESP_LOGI(TAG, "uart set pin");
    ESP_ERROR_CHECK(uart_set_pin(uart_num, -1, rx_pin, -1, -1));
    ESP_LOGI(TAG, "uart set line");
    ESP_ERROR_CHECK(uart_set_line_inverse(uart_num, UART_SIGNAL_RXD_INV));

    const int uart_buffer_size = max(UART_HW_FIFO_LEN(uart_num), ROUGH_P1_SIZE * 2);
    ESP_LOGI(TAG, "uart driver install");
    ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, 0, 0, NULL, __INTR_FLAGS));
}

void p1_init(const uart_port_t uart_num, gpio_num_t rx_pin, ringbuf_t target) {
    setup_uart(uart_num, rx_pin);

    struct config * ctx = malloc(sizeof(struct config));
    ctx->port = uart_num;
    ctx->target = target;

    xTaskCreate(read_serial_task, "uart_read_task", 2048, ctx, 10, NULL);
}