#pragma once

#include <stdint.h>
#include <memory.h>
#include "hal/uart_types.h"
#include "hal/gpio_types.h"
#include "ringbuf.h"


void p1_init(const uart_port_t port, gpio_num_t rx_pin, ringbuf_t target);