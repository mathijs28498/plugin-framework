#pragma once

typedef struct gfx_d2d_s gfx_d2d_t;

struct gfx_d2d_s {
    const gfx_vtable_t *vtable;
    int test_int;
};

void renderer_d2d_test();