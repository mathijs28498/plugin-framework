#pragma once

#include <stdint.h>

struct GuiApplicationContext;
struct WindowInterfaceCreateWindowOptions;

int32_t gui_application_default_setup(struct GuiApplicationContext *context, struct WindowInterfaceCreateWindowOptions *create_window_options);
int32_t gui_application_default_run(struct GuiApplicationContext *context);

