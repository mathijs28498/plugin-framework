#pragma once

#include <stdint.h>

struct DrawContext;

int32_t draw_default_start(struct DrawContext *context);
int32_t draw_default_present(struct DrawContext *context);