#include "common_buffer.h"
#include <memory.h>

common_buffer_t * allocate(size_t buffer_size) {
    common_buffer_t * result = malloc(sizeof(common_buffer_t));
    result->buffer = malloc(buffer_size);
    result->size = buffer_size;
    result->available = 0;
    result->cookie = 0;
    result->__lock = xSemaphoreCreateBinary();
    // initialize the lock
    xSemaphoreGive(result->__lock);
    return result;
}

void lock(common_buffer_t * buffer) {
    while (xSemaphoreTake(buffer->__lock, pdMS_TO_TICKS(10)) == pdFALSE);
}

void unlock(common_buffer_t * buffer) {
    xSemaphoreGive(buffer->__lock);
}