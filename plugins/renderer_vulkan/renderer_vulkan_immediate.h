#pragma once

#include <stdint.h>

#include <plugin_sdk/renderer/v1/renderer_types.h>

struct RendererContext;

int32_t renderer_vulkan_immediate_execute(struct RendererContext *context, ImmediateExecute_Fn immediate_execute_fn, void *user_data);