#pragma once

#include <stdint.h>
#include <stddef.h>

struct ArenaAllocatorContext;

int32_t arena_allocator_default_get_arena(struct ArenaAllocatorContext *context, size_t size, void **out_arena);
void arena_allocator_default_set_memory_pool(struct ArenaAllocatorContext *context, uint8_t *memory_pool, const size_t memory_pool_size);
void arena_allocator_default_freeze_permanent_arenas(struct ArenaAllocatorContext *context);