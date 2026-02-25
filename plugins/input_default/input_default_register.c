#include "input_default_register.h"

#include <plugin_sdk.h>
#include <input_interface.h>
#include <logger_interface.h>

#include "input_default.h"

#define PLUGIN_INTERFACE_NAME input

InputInterface *get_interface(void)
{
    static InputInterfaceContext context = {0};

    static InputInterface iface = {
        .context = &context,
        .prepare_processing = input_default_prepare_processing,
        .process_window_event = input_default_process_window_event,
        .key_pressed = input_default_key_pressed,
        .key_held = input_default_key_held,
        .key_released = input_default_key_released,
    };

    return &iface;
}

PLUGIN_REGISTER_INTERFACE(get_interface, InputInterface)