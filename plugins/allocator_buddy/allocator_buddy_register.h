#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "plugin_dependencies.h"

#pragma pack(push, 8)

typedef struct AllocatorContext
{
    PluginDependencies deps;

    uint8_t page_size_msb;

    uint8_t *free_list_tree;
    uint8_t free_list_depth;
    uint8_t *memory_pool;
    uint64_t memory_pool_size;
    bool permanent_allocations_frozen;
} AllocatorContext;

#pragma pack(pop)

