#include "input_default_register.h"

#include <input_interface.h>

#include "input_default.h"

static const InputVtable plugin_vtable = {
    .key_held = input_default_key_held,
    .key_pressed = input_default_key_pressed,
    .key_released = input_default_key_released,
    .prepare_processing = input_default_prepare_processing,
    .process_window_event = input_default_process_window_event,
};

#include "plugin_register.c.inc"
