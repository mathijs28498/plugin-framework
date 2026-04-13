#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <plugin_utils.h>

TODO("Use an arena allocator of 4MB for the startup")
#define MAX_EXTENSIONS_LEN 512
#define MAX_QUEUE_CREATE_INFO_ARR_LEN 64
#define MAX_PHYSICAL_DEVICES_LEN 64
#define MAX_QUEUE_FAMILIES_LEN 64
#define MAX_PHYSICAL_DEVICE_EXTENSIONS_LEN 512

#if IS_DEBUG
TODO("Make this a setting for plugin")
#define MAX_INSTANCE_LAYER_PROPERTIES_LEN 64
#endif // #if IS_DEBUG

typedef struct QueueFamilyIndices
{
    bool has_graphics_family;
    uint32_t graphics_family;

    bool has_present_family;
    uint32_t present_family;
} QueueFamilyIndices;



struct RendererContext;

int32_t renderer_vulkan_bootstrap(struct RendererContext *context);
int32_t renderer_vulkan_bootstrap_cleanup(struct RendererContext *context);