#include "window_win32_register.h"

#include <stdint.h>
#include <Windows.h>

#include <plugin_sdk.h>
#include <environment_interface.h>
#include <window_interface.h>
#include <logger_interface.h>

#include "window_win32.h"
#include "window_win32_window_events.h"

#define PLUGIN_DEPENDENCIES(X)                        \
    X(EnvironmentInterface, environment, environment) \
    X(LoggerInterface, logger, logger)

PLUGIN_REGISTER_DEPENDENCIES(WindowInterfaceContext, PLUGIN_DEPENDENCIES);

WindowInterface *get_interface()
{
    static WindowInterfaceContext context = {0};

    static WindowInterface iface = {
        .context = &context,

        .create_window = window_win32_create_window,
        .close_window = window_win32_close_window,

        .poll_os_events = window_win32_poll_os_events,
        .wait_for_os_events = window_win32_wait_for_os_events,
        .pop_window_event = window_win32_window_events_pop,
    };

    return &iface;
}

PLUGIN_REGISTER_INTERFACE(get_interface, WindowInterface);