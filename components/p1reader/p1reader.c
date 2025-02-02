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
    common_buffer_t *target;
    uart_port_t port;
};


static void read_serial_task(void *arg) {
    struct config const * ctx = arg;
    while(true) {
        lock(ctx->target);
        // ok, we got the lock
        int read = uart_read_bytes(ctx->port, ctx->target->buffer, ctx->target->size, pdMS_TO_TICKS(200));
        if (read > 0) {
            ctx->target->available = read;
            ctx->target->cookie++;
        }
        unlock(ctx->target);
        vTaskDelay(pdMS_TO_TICKS(100)); // leave room for the reader to pick up the lock
    }
}


#if CONFIG_UART_ISR_IN_IRAM
    #define __INTR_FLAGS ESP_INTR_FLAG_IRAM
#else
    #define __INTR_FLAGS 0
#endif

#define ROUGH_P1_SIZE 1024

static void setup_uart(const uart_port_t uart_num, gpio_num_t rx_pin) {
    ESP_LOGI(TAG, "Starting serial setup UART %d on GPIO %d", uart_num, rx_pin);
    // Configure UART parameters
    ESP_LOGI(TAG, "configure param");

    uart_config_t p1_serial_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    ESP_ERROR_CHECK(uart_param_config(uart_num, &p1_serial_config));
    ESP_LOGI(TAG, "set pin");
    ESP_ERROR_CHECK(uart_set_pin(uart_num, -1, rx_pin, -1, -1));
    ESP_LOGI(TAG, "set inverted line");
    ESP_ERROR_CHECK(uart_set_line_inverse(uart_num, UART_SIGNAL_RXD_INV));

    const int uart_buffer_size = max(UART_HW_FIFO_LEN(uart_num), ROUGH_P1_SIZE * 2);
    ESP_LOGI(TAG, "driver install");
    ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, 0, 0, NULL, __INTR_FLAGS));
    ESP_LOGI(TAG, "ready to start reading");
}

void p1_start(const uart_port_t uart_num, gpio_num_t rx_pin, common_buffer_t* target) {
    setup_uart(uart_num, rx_pin);

    struct config * ctx = malloc(sizeof(struct config));
    ctx->port = uart_num;
    ctx->target = target;

    xTaskCreate(read_serial_task, "uart_read_task", 2048, ctx, 10, NULL);
}