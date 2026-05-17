#pragma once

#include <stdint.h>

struct DrawContext;
struct RendererCommandList;

int32_t draw_default_render_frame(struct DrawContext *context, struct RendererCommandList *command_list);