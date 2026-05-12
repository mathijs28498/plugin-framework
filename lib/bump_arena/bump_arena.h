#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <plugin_sdk/plugin_utils.h>

typedef struct BumpArenaCheckpoint
{
    size_t offset;
} BumpArenaCheckpoint;

int32_t bump_arena_alloc(uint8_t *arena_a, size_t size, size_t alignment, void *out_allocated);
void bump_arena_free(uint8_t *arena_a, bool set_zero);

BumpArenaCheckpoint bump_arena_create_checkpoint(uint8_t *arena_a);
void bump_arena_restore_checkpoint(uint8_t *arena_a, BumpArenaCheckpoint checkpoint, bool set_zero);

#define BUMP_ARENA_ALLOC_TYPED(arena_a, type, amount, out_allocated) bump_arena_alloc((arena_a), sizeof(type) * (amount), _Alignof(type), (out_allocated))
#define BUMP_ARENA_ALLOC_GENERIC(arena_a, size, out_allocated) bump_arena_alloc((arena_a), (size), MAX_ALIGN, (out_allocated))
