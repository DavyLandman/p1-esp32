#pragma once

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"


void server_start(uint16_t port, StreamBufferHandle_t source);