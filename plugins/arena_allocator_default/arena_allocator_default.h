#pragma once

#include <stdint.h>

struct ArenaAllocatorContext;

int32_t arena_allocator_default_get_arena(struct ArenaAllocatorContext *context, uint64_t size, uint8_t **out_arena);
void arena_allocator_default_set_memory_pool(struct ArenaAllocatorContext *context, uint8_t *memory_pool, const uint64_t memory_pool_size);