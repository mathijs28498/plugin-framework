#pragma once

#include <stdint.h>

struct RendererContext;
struct RendererCommandList;

int32_t renderer_vulkan_render_begin_frame(struct RendererContext *context, struct RendererCommandList **out_command_list);
int32_t renderer_vulkan_render_end_frame(struct RendererContext *context);
