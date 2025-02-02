#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdint.h>

typedef struct {
    char *buffer;
    size_t size;
    size_t available;
    unsigned int cookie;
    SemaphoreHandle_t __lock;
} common_buffer_t;

common_buffer_t * allocate(size_t buffer_size);

void lock(common_buffer_t * buffer);
void unlock(common_buffer_t * buffer);
