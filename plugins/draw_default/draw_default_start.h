#pragma once

#include <stdint.h>

struct DrawContext;

int32_t draw_default_create_draw_image(struct DrawContext *context);
int32_t draw_default_start(struct DrawContext *context);
void draw_default_cleanup(struct DrawContext *context);