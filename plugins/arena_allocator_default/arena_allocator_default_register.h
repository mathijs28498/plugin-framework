#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "plugin_dependencies.h"

#pragma pack(push, 8)

typedef struct ArenaAllocatorContext
{
    PluginDependencies deps;

    uint8_t *memory_pool;
    uint64_t memory_pool_size;
    bool permanent_arenas_frozen;
} ArenaAllocatorContext;

#pragma pack(pop)

