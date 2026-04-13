#pragma once

#include <stdint.h>

struct RendererContext;

int32_t renderer_vulkan_start(struct RendererContext *context);
int32_t renderer_vulkan_start_cleanup(struct RendererContext *context);