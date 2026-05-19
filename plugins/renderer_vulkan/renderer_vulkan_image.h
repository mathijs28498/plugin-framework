#pragma once

#include <stdint.h>

#include <plugin_sdk/renderer/v1/renderer_types.h>

struct RendererContext;
struct RendererImageCreateInfo;
struct RendererImageProperties;

int32_t renderer_vulkan_create_image(struct RendererContext *context, struct RendererImageCreateInfo *renderer_image_create_info, RendererImageHandle *out_image_handle);
int32_t renderer_vulkan_destroy_image(struct RendererContext *context, RendererComputePipelineHandle pipeline_handle);
RendererImageHandle renderer_vulkan_get_render_image_handle(struct RendererContext *context);
int32_t renderer_vulkan_get_image_properties(struct RendererContext *context, RendererImageHandle image_handle, struct RendererImageProperties *out_image_properties);