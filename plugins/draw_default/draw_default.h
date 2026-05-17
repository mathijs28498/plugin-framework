#pragma once

#include <stdint.h>

struct DrawContext;

int32_t draw_default_present(struct DrawContext *context);
void draw_default_on_window_resize(struct DrawContext *context, uint32_t width, uint32_t height);
void draw_default_cleanup(struct DrawContext *context);