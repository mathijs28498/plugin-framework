#include "renderer_vulkan_plugin_register.h"

#include <stdint.h>

#include <logger_api.h>
#include <plugin_manager_impl.h>
#include <renderer_api.h>

#include "renderer_vulkan_plugin.h"

#define PLUGIN_API_NAME renderer_api

#define REGISTER_DEPENDENCIES(X) \
    X(LoggerApi, logger_api, logger_api)

PLUGIN_REGISTER_DEPENDENCIES(RendererApiContext, REGISTER_DEPENDENCIES);

RendererApi *get_api()
{
    static RendererApiContext context = {0};

    static RendererApi api = {
        .context = &context,
    };

    return &api;
}

PLUGIN_REGISTER_API(get_api, RendererApi);