#include "draw_plugin_register.h"

#include <stdint.h>

#include <logger_api.h>
#include <plugin_manager_impl.h>
#include <draw_api.h>

#include "draw_plugin.h"

#define PLUGIN_API_NAME draw_api

#define REGISTER_DEPENDENCIES(X) \
    X(LoggerApi, logger_api, logger_api)

PLUGIN_REGISTER_DEPENDENCIES(DrawApiContext, REGISTER_DEPENDENCIES);

DrawApi *get_api()
{
    static DrawApiContext context = {0};

    static DrawApi api = {
        .context = &context,

        .present = draw_plugin_present,
    };

    return &api;
}

PLUGIN_REGISTER_API(get_api, DrawApi);