#pragma once

#include <stdint.h>
#include <memory.h>
#include "hal/uart_types.h"
#include "hal/gpio_types.h"
#include "common_buffer.h"


void p1_start(const uart_port_t port, gpio_num_t rx_pin, common_buffer_t *target);