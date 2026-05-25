#pragma once

#include <stdint.h>

#include <plugin_sdk/renderer/v1/renderer_types.h>

struct RendererContext;
struct RendererBufferCreateInfo;

int32_t renderer_vulkan_create_buffer(struct RendererContext *context, struct RendererBufferCreateInfo *renderer_buffer_create_info, RendererBufferHandle *out_buffer_handle);