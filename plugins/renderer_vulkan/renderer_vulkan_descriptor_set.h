#pragma once

#include <stdint.h>

struct RendererContext;

int32_t create_descriptor_sets(struct RendererContext *context);
void renderer_vulkan_update_descriptor_set(struct RendererContext *context);