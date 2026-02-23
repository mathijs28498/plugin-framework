#include "input_plugin_register.h"

#include <plugin_manager_impl.h>
#include <input_interface.h>

#include "input_plugin.h"

#define PLUGIN_INTERFACE_NAME input

InputInterface *get_interface(void)
{
    static InputInterfaceContext context = {0};

    static InputInterface iface = {
        .context = &context,
        .prepare_processing = input_plugin_prepare_processing,
        .process_window_event = input_plugin_process_window_event,
        .key_pressed = input_plugin_key_pressed,
        .key_held = input_plugin_key_held,
        .key_released = input_plugin_key_released,
    };

    return &iface;
}

PLUGIN_REGISTER_INTERFACE(get_interface, InputInterface)