#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "renderer_d2d.h"
#include "renderer_d2d_resources.h"
#include "../gfx.h"

int gfx_create(gfx_t *gfx)
{
    HRESULT res;

    res = CoInitialize(NULL);

    res = rdr_create_device_indepedent_resources();

    gfx->ctx = malloc(sizeof(gfx_d2d_t));

    return 0;
}

int gfx_destroy(gfx_t *gfx)
{
    free(gfx->ctx);
    return 0;
}

void renderer_d2d_test()
{
    printf("Inside renderer\n");
}