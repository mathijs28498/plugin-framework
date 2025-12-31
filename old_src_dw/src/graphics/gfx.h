typedef struct gfx_s gfx_t;
typedef struct gfx_vtable_s gfx_vtable_t;

struct gfx_s
{
    const gfx_vtable_t *vtable;
    void *ctx;
};

struct gfx_vtable_s
{
    int (*test)(const void *ctx);
    int (*update_text)(const void *ctx, char* text);
    int (*render_text)(const void *ctx);
};

int gfx_create(gfx_t *gfx);

int gfx_destroy(gfx_t *gfx);