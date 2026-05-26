#pragma once

#include <stdint.h>

#include "renderer_vulkan_utils.h"

struct RendererContext;

int32_t renderer_vulkan_start(struct RendererContext *context);
int32_t renderer_vulkan_start_recreate_swapchain(struct RendererContext *context);
