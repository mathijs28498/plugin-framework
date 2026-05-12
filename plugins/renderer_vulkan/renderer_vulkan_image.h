#pragma once

#include <stdint.h>

typedef uint64_t RendererImageHandle;

struct RendererContext;
struct RendererImageCreateInfo;

int32_t renderer_vulkan_create_image(struct RendererContext *context, struct RendererImageCreateInfo *renderer_image_create_info, RendererImageHandle *out_image_handle);