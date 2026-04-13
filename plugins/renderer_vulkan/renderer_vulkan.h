#pragma once

#include <stdint.h>

struct RendererContext;

int32_t renderer_vulkan_render(struct RendererContext *context);
int32_t renderer_vulkan_cleanup(struct RendererContext *context);