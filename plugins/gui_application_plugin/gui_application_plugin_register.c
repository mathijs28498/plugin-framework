#include "gui_application_plugin_register.h"

#include <gui_application_api.h>
#include <plugin_manager_impl.h>
#include <logger_api.h>
#include <window_api.h>
#include <input_api.h>
#include <draw_api.h>

#include "gui_application_plugin.h"

// REG
#define PLUGIN_API_NAME gui_application_api

#define PLUGIN_DEPENDENCIES(X)           \
    X(LoggerApi, logger_api, logger_api) \
    X(WindowApi, window_api, window_api) \
    X(DrawApi, draw_api, draw_api) \
    X(InputApi, input_api, input_api)

PLUGIN_REGISTER_DEPENDENCIES(GuiApplicationApiContext, PLUGIN_DEPENDENCIES);

GuiApplicationApi *get_api(void)
{
    static GuiApplicationApiContext context = {0};

    static GuiApplicationApi api = {
        .context = &context,

        .run = gui_application_plugin_run,
    };

    return &api;
}

PLUGIN_REGISTER_API(get_api, GuiApplicationApi);