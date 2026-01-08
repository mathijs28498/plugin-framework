#include <stdio.h>
#include <stdint.h>

#include <renderer_vk/renderer_vk.h>

#include "renderer.h"

int32_t renderer_init()
{
    return vk_init();
}

int32_t renderer_cleanup()
{
    return vk_cleanup();
}