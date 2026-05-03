#pragma once

#include <stdint.h>

struct RendererContext;
struct RendererCommandList;

void renderer_vulkan_cmd_begin_render_pass(struct RendererContext *context, struct RendererCommandList *command_list);
void renderer_vulkan_cmd_end_render_pass(struct RendererContext *context, struct RendererCommandList *command_list);
void renderer_vulkan_cmd_bind_pipeline(struct RendererContext *context, struct RendererCommandList *command_list, uint32_t pipeline_handle);
void renderer_vulkan_cmd_draw(struct RendererContext *context, struct RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);