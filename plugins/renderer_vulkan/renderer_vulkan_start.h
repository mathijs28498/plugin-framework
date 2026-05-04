#pragma once

#include <stdint.h>

#include "renderer_vulkan_utils.h"

RV_CREATE_HANDLE_DEFINITION(VkCommandPool);
RV_CREATE_HANDLE_DEFINITION(VkCommandBuffer);

typedef struct RendererStartContext {
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;

    struct RV_CallRecord *destroy_queue;
} RendererStartContext;

struct RendererContext;

int32_t renderer_vulkan_start(struct RendererContext *context);
int32_t renderer_vulkan_start_recreate_swapchain(struct RendererContext *context);