#include "arena_allocator_default.h"

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(arena_allocator_default, LOG_LEVEL_DEBUG)
#include <plugin_sdk/arena_allocator/v1/arena_allocator_interface.h>
#include <plugin_sdk/arena_allocator/v1/arena_allocator_pm_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include "arena_allocator_default_register.h"

#define ARENA_PAGE_SIZE 128

#define ALIGN_WITH_ARENA_PAGE(value) \
    (((value) + ((uintptr_t)(ARENA_PAGE_SIZE) - 1)) & ~((uintptr_t)(ARENA_PAGE_SIZE) - 1))

int32_t allocate_permanent_arena(ArenaAllocatorContext *context, size_t size, void **out_arena)
{
    size_t occupied_size = ALIGN_WITH_ARENA_PAGE(size);

    if (context->memory_pool_size < occupied_size)
    {
        LOG_ERR_TRACE(context->deps.logger, "Not enough space for allocating permanent arena: requested '%zu', size '%zu'", occupied_size, context->memory_pool_size);
        return -1;
    }

    memset(context->memory_pool, 0, occupied_size);

    *out_arena = context->memory_pool;
    context->memory_pool += occupied_size;
    context->memory_pool_size -= occupied_size;

    return 0;
}

TODO("Get arena, keep track of what arenas are taken out");
TODO("Allow user to give an id that needs to be registered first");
int32_t arena_allocator_default_get_arena(ArenaAllocatorContext *context, size_t size, void **out_arena)
{
    assert(context != NULL);
    assert(size > 0);
    assert(out_arena != NULL);

    if (context->permanent_arenas_frozen)
    {
        LOG_ERR_TRACE(context->deps.logger, "Non permanent frozen arenas are currently not supported");
        return -1;
    }

    return allocate_permanent_arena(context, size, out_arena);
}

void arena_allocator_default_set_memory_pool(ArenaAllocatorContext *context, uint8_t *memory_pool, const size_t memory_pool_size)
{
    assert(context != NULL);
    assert(memory_pool != NULL);

    uintptr_t current_addr = (uintptr_t)memory_pool;
    uintptr_t aligned_addr = ALIGN_WITH_ARENA_PAGE(current_addr);
    size_t padding = aligned_addr - current_addr;

    context->memory_pool = (uint8_t *)aligned_addr;
    context->memory_pool_size = memory_pool_size - padding;
}

void arena_allocator_default_freeze_permanent_arenas(ArenaAllocatorContext *context)
{
    assert(context != NULL);

    context->permanent_arenas_frozen = true;
}