#include "draw_2d_plugin_register.h"

#include <stdint.h>

#include <logger_api.h>
#include <plugin_manager_impl.h>
#include <draw_2d_api.h>

#include "draw_2d_plugin.h"

#define PLUGIN_API_NAME draw_2d_api

#define REGISTER_DEPENDENCIES(X) \
    X(LoggerApi, logger_api, logger_api)

PLUGIN_REGISTER_DEPENDENCIES(Draw2dApiContext, REGISTER_DEPENDENCIES);

Draw2dApi *get_api()
{
    static Draw2dApiContext context = {0};

    static Draw2dApi api = {
        .context = &context,
    };

    return &api;
}

PLUGIN_REGISTER_API(get_api, Draw2dApi);