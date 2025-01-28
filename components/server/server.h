#pragma once

#include <stdint.h>

#include "ringbuf.h"

void server_start(uint16_t port, ringbuf_t source);