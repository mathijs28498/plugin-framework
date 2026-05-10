#include "bump_arena.h"

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include <plugin_sdk/plugin_utils.h>

int32_t bump_arena_alloc(uint8_t *arena_a, size_t size, size_t alignment, void *out_allocated)
{
    assert(arena_a != NULL);
    assert(out_allocated != NULL);
    assert(size > 0);
    assert(alignment > 0);
    assert((alignment & (alignment - 1)) == 0);

    uintptr_t allocated_unaligned = (uintptr_t)arena_a + (uintptr_t)GET_ARRAY_LENGTH(arena_a);
    uintptr_t allocated_aligned = (allocated_unaligned + alignment - 1) & ~(alignment - 1);

    if (allocated_aligned < allocated_unaligned)
    {
        return -1;
    }

    if (size > (uintptr_t)arena_a + (uintptr_t)GET_ARRAY_CAPACITY(arena_a) - allocated_aligned)
    {
        return -2;
    }

    GET_ARRAY_LENGTH(arena_a) += (size_t)(size + (allocated_aligned - allocated_unaligned));
    *(void **)out_allocated = (void *)allocated_aligned;
    return 0;
}

void bump_arena_free(uint8_t *arena_a, bool set_zero)
{
    assert(arena_a != NULL);
    if (set_zero)
    {
        memset(arena_a, 0, GET_ARRAY_LENGTH(arena_a));
    }
    GET_ARRAY_LENGTH(arena_a) = 0;
}

BumpArenaCheckpoint bump_arena_create_checkpoint(uint8_t *arena_a)
{
    assert(arena_a != NULL);

    return (BumpArenaCheckpoint){.offset = GET_ARRAY_LENGTH(arena_a)};
}

void bump_arena_restore_checkpoint(uint8_t *arena_a, BumpArenaCheckpoint checkpoint, bool set_zero)
{
    assert(arena_a != NULL);
    assert(checkpoint.offset <= GET_ARRAY_LENGTH(arena_a));

    if (set_zero)
    {
        memset(arena_a + checkpoint.offset, 0, GET_ARRAY_LENGTH(arena_a) - checkpoint.offset);
    }
    GET_ARRAY_LENGTH(arena_a) = checkpoint.offset;
}