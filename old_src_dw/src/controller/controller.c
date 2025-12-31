#include <stdio.h>

#include "controller.h"

#warning TODO: Never use the graphics_d2d directly. the gfx header should determine which backend to use
#include "../graphics/graphics_d2d/renderer_d2d.h"
// #include "../graphics/renderer_d2d.h"
#include "../logic/text_editor.h"

void controller_test()
{
    printf("Inside controller test\n");

    renderer_d2d_test();
    text_editor_test();
}