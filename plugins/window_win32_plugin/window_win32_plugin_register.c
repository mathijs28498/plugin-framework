#include "window_win32_plugin_register.h"

#include <stdint.h>
#include <Windows.h>

#include <plugin_manager_impl.h>
#include <environment_api.h>
#include <window_api.h>
#include <logger_api.h>
LOGGER_API_REGISTER(window_win32_plugin, LOG_LEVEL_DEBUG)

#include "window_win32_plugin.h"

#define PLUGIN_API_NAME window_api

#define PLUGIN_DEPENDENCIES(X)                          \
    X(EnvironmentApi, environment_api, environment_api) \
    X(LoggerApi, logger_api, logger_api)

PLUGIN_REGISTER_DEPENDENCIES(WindowApiContext, PLUGIN_DEPENDENCIES);

WindowApi *get_api()
{
    static WindowApiContext context = {0};

    static WindowApi api = {
        .context = &context,

        .create_window = window_win32_plugin_create_window,
        .poll_events = window_win32_plugin_poll_events,
        .wait_for_events = window_win32_plugin_wait_for_events,
    };

    return &api;
}

PLUGIN_REGISTER_API(get_api, WindowApi);