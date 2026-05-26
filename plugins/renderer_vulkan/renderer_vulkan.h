#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef union
{
    struct
    {
        uint32_t generation;
        uint32_t index;
    };
    uint64_t raw;
} RendererVulkanHandle;

struct RendererContext;

int32_t renderer_vulkan_cleanup(struct RendererContext *context);
void renderer_vulkan_on_window_resize(struct RendererContext *context, uint32_t width, uint32_t height);
bool renderer_vulkan_consume_has_resized(struct RendererContext *context);