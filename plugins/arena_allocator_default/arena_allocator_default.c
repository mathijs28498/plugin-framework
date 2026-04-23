#include "arena_allocator_default.h"

#include <assert.h>
#include <stdint.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(arena_allocator_default, LOG_LEVEL_DEBUG)
#include <plugin_sdk/arena_allocator/v1/arena_allocator_interface.h>
#include <plugin_sdk/arena_allocator/v1/arena_allocator_pm_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include "arena_allocator_default_register.h"

int32_t arena_allocator_default_get_arena(ArenaAllocatorContext *context, uint64_t size, uint8_t **out_arena )
{
    assert(context != NULL);
    assert(size > 0);
    assert(out_arena != NULL);

    TODO("Get arena, keep track of what arenas are taken out");
    TODO("Allow user to give an id that needs to be registered first");

    return 0;
}

void arena_allocator_default_set_memory_pool(ArenaAllocatorContext *context, uint8_t *memory_pool, const uint64_t memory_pool_size)
{
    assert(context != NULL);
    assert(memory_pool != NULL);

    context->memory_pool = memory_pool;
    context->memory_pool_size = memory_pool_size;
}