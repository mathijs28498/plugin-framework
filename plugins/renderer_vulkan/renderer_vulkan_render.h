#pragma once

#include <stdint.h>

struct RendererContext;

int32_t renderer_vulkan_render_begin_frame(struct RendererContext *context);
int32_t renderer_vulkan_render_end_frame(struct RendererContext *context);
int32_t renderer_vulkan_render(struct RendererContext *context);