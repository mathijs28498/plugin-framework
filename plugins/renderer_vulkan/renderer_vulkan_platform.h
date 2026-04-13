#pragma once

#include <stdint.h>

#include "renderer_vulkan_utils.h"

struct RendererContext;

CREATE_VK_HANDLE_DEFINITION(VkSurfaceKHR)

int32_t renderer_vulkan_platform_create_surface(struct RendererContext *context, VkSurfaceKHR *surface);
void renderer_vulkan_platform_get_required_extensions(const char ***out_extensions);