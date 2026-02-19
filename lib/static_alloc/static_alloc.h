#pragma once

#include <stdint.h>

void *static_malloc(uint32_t size);
void static_free(void *ptr);