#include "server.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "esp_system.h"
//#include "esp_wifi.h"
//#include "esp_event.h"
#include "esp_log.h"
//#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
//#include "lwip/sys.h"
//#include <lwip/netdb.h>

static const char *TAG = "tcp-server";


struct config {
    ringbuf_t source;
    uint16_t port;
};

#define LWIF_ERROR_CHECK(__err, __message) {    \
    if (__err < 0) { \
        ESP_LOGE(TAG, __message" (error: %d)", errno); \
        abort(); \
        return; \
    } \
}

#define LWIF_SOCK_OPT(sock, target, variable, value) { \
    int __opt = value; \
    setsockopt(sock, target, variable, &__opt, sizeof(int)); \
}

static void configure_socket(int socket) {
    LWIF_SOCK_OPT(socket, SOL_SOCKET, SO_KEEPALIVE, 1)
    LWIF_SOCK_OPT(socket, IPPROTO_TCP, TCP_KEEPIDLE, 3)
    LWIF_SOCK_OPT(socket, IPPROTO_TCP, TCP_KEEPINTVL, 5)
    LWIF_SOCK_OPT(socket, IPPROTO_TCP, TCP_KEEPCNT, 5)
}

static const size_t IDEAL_PACKET_SIZE = 1024;

static void tcp_server_task(struct config const *ctx) {
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP); // TODO: shouldn't it be _TCP?
    LWIF_ERROR_CHECK(server_socket,  "Unable to create socket")
    ESP_LOGI(TAG, "Socket created");

    struct sockaddr_in addr = {
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_family = AF_INET,
        .sin_port = htons(ctx->port)
    };

    int err = bind(server_socket, &addr, sizeof(addr));
    LWIF_ERROR_CHECK(err, "Unable to bind socket")
    ESP_LOGI(TAG, "Socket bound to address");

    err = listen(server_socket, 1);
    LWIF_ERROR_CHECK(err, "Unable to listen socket")

    uint8_t* buffer = malloc(IDEAL_PACKET_SIZE);

    while (true) {
        ESP_LOGI(TAG, "Socket listening for new connection");
        struct sockaddr_in source_addr;
        int client_sock = accept(server_socket, &source_addr, sizeof(source_addr));
        LWIF_ERROR_CHECK(client_sock, "Could not accept a connection");

        configure_socket(client_sock);
        while (true) {
            uint8_t const* written_end = ringbuf_memcpy_from(ctx->source, buffer, IDEAL_PACKET_SIZE);
            if (written_end == NULL) {
                // buffer was empty
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
            }
            size_t to_send = written_end - buffer;
            uint8_t const* data = buffer;
            bool disconnected = false;
            while (to_send > 0) {
                ssize_t current_send = send(client_sock, data, to_send, 0);
                if (current_send <= 0) {
                    if (errno == ECONNABORTED || errno == ECONNRESET || errno == ETIMEDOUT || current_send == 0) {
                        disconnected = true;
                        ESP_LOGI(TAG, "Connection got disconnected (error: %d)", errno);
                    }
                    else {
                        ESP_LOGE(TAG, "Error forwarding data (error: %d)", errno);
                    }
                    break;
                }
                to_send -= current_send;
                data += current_send;
            }
            if (disconnected) {
                break;
            }
        }
        ESP_LOGI(TAG, "closing client socket: %d", client_sock);
        shutdown(client_sock, 0);
        close(client_sock);
    }


}

void server_start(uint16_t port, ringbuf_t source) {
    struct config * config = malloc(sizeof(struct config));
    config->port = port;
    config->source = source;
    xTaskCreate(tcp_server_task, "tcp_server", 4096, config, 5, NULL);
}
