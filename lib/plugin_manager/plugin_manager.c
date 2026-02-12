#include "plugin_manager.h"

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <cjson.h>
#include <Windows.h>
#include <assert.h>

// TODO: Change the name of this file
#include <plugin_manager_api.h>

#include <environment_api.h>
#include <environment_plugin.h>
#include <logger_api.h>
#include <logger_plugin.h>

#include "file_io.h"
#include "plugin_registry.h"
#include "plugin_manager_types.h"
#include "plugin_manager_loader.h"

#define LOGGER_API_TAG "plugin_manager"

#define PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH 255

PluginManagerRuntimeContext *get_plugin_manager_runtime_context()
{
    static PluginManagerRuntimeContext context = {0};
    return &context;
}

int32_t plugin_manager_init(PluginManagerSetupContext **setup_context, int argc, char **argv)
{
    assert(setup_context != NULL);

    PluginManagerSetupContext *new_setup_context = calloc(1, sizeof(**setup_context));
    if (new_setup_context == NULL)
    {
        return -1;
    }

    {
        assert(new_setup_context->internal_plugins_len < sizeof(new_setup_context->internal_plugins) / sizeof(new_setup_context->internal_plugins[0]));

        EnvironmentApi *environment_api = environment_api_get_api();
        environment_plugin_set_args(environment_api->context, argc, argv);

        PluginStatic environment_plugin = {
            .api_name = "environment_api",
            .plugin_name = "internal_environment_plugin",
            .dependencies_len = 0,
            .api = (PluginManagerBaseApi *)environment_api,
        };

        memcpy(&new_setup_context->internal_plugins[new_setup_context->internal_plugins_len],
               &environment_plugin,
               sizeof(new_setup_context->internal_plugins[new_setup_context->internal_plugins_len]));
        new_setup_context->internal_plugins_len++;
    }

    {
        assert(new_setup_context->internal_plugins_len < sizeof(new_setup_context->internal_plugins) / sizeof(new_setup_context->internal_plugins[0]));

        LoggerApi *logger_api = logger_api_get_api();
        new_setup_context->logger_api = logger_api;

        PluginStatic logger_plugin = {
            .api_name = "logger_api",
            .plugin_name = "internal_logger_plugin",
            .dependencies_len = 0,
            .api = (PluginManagerBaseApi *)logger_api,
        };

        memcpy(&new_setup_context->internal_plugins[new_setup_context->internal_plugins_len],
               &logger_plugin,
               sizeof(new_setup_context->internal_plugins[new_setup_context->internal_plugins_len]));
        new_setup_context->internal_plugins_len++;
    }
    *setup_context = new_setup_context;
    return 0;
}

int32_t plugin_manager_add(PluginManagerSetupContext *setup_context, const char *api_name, const char *plugin_name)
{
    if (api_name == NULL)
    {
        LOG_ERR(setup_context->logger_api, "Api name is NULL");
        return -1;
    }

    if (setup_context->requested_plugins_len >= PLUGIN_MANAGER_MAX_PLUGINS_LEN)
    {
        LOG_ERR(setup_context->logger_api, "Cannot add plugin as max plugin_definitions is reached. Max plugin count \"%d\"", PLUGIN_MANAGER_MAX_PLUGINS_LEN);
        return -1;
    }

    snprintf(setup_context->requested_plugins[setup_context->requested_plugins_len].api_name, PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN,
             "%s", api_name);

    if (plugin_name == NULL)
    {
        setup_context->requested_plugins[setup_context->requested_plugins_len].plugin_name[0] = '\0';
    }
    else
    {
        snprintf(setup_context->requested_plugins[setup_context->requested_plugins_len].plugin_name, PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN,
                 "%s", plugin_name);
    }

    setup_context->requested_plugins[setup_context->requested_plugins_len].resolved = false;

    setup_context->requested_plugins_len++;
    return 0;
};

// TODO: plugin add functionality
//          [X] - Load in plugin_definitions to add
//          [X] - Call dependency functions on plugin_definitions to add
//          [X] - Check if dependencies are or are not included
//          [X] - Add dependencies where not already included
//          [X] - Repeat this until all dependencies loaded in
//          [ ] - Create dependency graph
//              [ ] - Do error checking on these dependencies (eg. cyclic dependencies)
//          [/] - Walk dependency graph layer by layer calling the init functions

int32_t plugin_manager_load(PluginManagerSetupContext *setup_context, PluginManagerRuntimeContext *runtime_context)
{
    int ret;
    char *buffer;
    PluginRegistry plugin_registry;
    (void)runtime_context;

    // TODO: Make buffer not use malloc
    ret = file_io_read("../plugin_registry.json", &buffer);
    ret = plugin_registry_deserialize_json(buffer, &plugin_registry);
    free(buffer);

    if (ret < 0)
    {
        LOG_ERR(setup_context->logger_api, "unable to parse plugin config: %d", ret);
        return ret;
    }

    size_t plugin_modules_len = 0;
    PluginModule plugin_modules[PLUGIN_MANAGER_MAX_PLUGINS_LEN];

    int i = 0;
    while (setup_context->requested_plugins_len > 0 && i < PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH)
    {
        ret = resolve_requested_plugins_registry(
            setup_context->logger_api,
            setup_context->requested_plugins,
            setup_context->requested_plugins_len,
            &plugin_registry,
            plugin_modules,
            &plugin_modules_len);

        if (ret < 0)
        {
            return ret;
        }

        ret = load_plugin_modules(
            setup_context->logger_api,
            plugin_modules,
            plugin_modules_len,
            runtime_context->api_instances,
            &runtime_context->api_instances_len);

        if (ret < 0)
        {
            return ret;
        }

        ret = resolve_requested_plugins_internal(
            setup_context->logger_api,
            setup_context->requested_plugins,
            setup_context->requested_plugins_len,
            setup_context->internal_plugins,
            setup_context->internal_plugins_len,
            runtime_context->api_instances,
            &runtime_context->api_instances_len);

        if (ret < 0)
        {
            return ret;
        }

        setup_context->requested_plugins_len = 0;

        ret = resolve_plugin_module_dependencies(
            setup_context->logger_api,
            runtime_context->api_instances,
            runtime_context->api_instances_len,
            plugin_modules,
            plugin_modules_len,
            setup_context->requested_plugins,
            &setup_context->requested_plugins_len);

        if (ret < 0)
        {
            return ret;
        }

        i++;
    }

    // TODO: Make sure the plugin_definitions get initialized in order with their dependencies

    uint32_t sorted_plugin_modules_indices[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
    ret = calculate_plugin_module_initialization_order(setup_context->logger_api, plugin_modules, plugin_modules_len, sorted_plugin_modules_indices);
    if (ret < 0)
    {
        return ret;
    }

    ret = initialize_plugins(setup_context->logger_api, sorted_plugin_modules_indices, plugin_modules, plugin_modules_len);
    if (ret < 0)
    {
        return ret;
    }

    free(setup_context);

    return 0;
}

int32_t plugin_manager_get(const PluginManagerRuntimeContext *runtime_context, const char *api_name, void **api_interface)
{
    (void)runtime_context;
    (void)api_name;
    printf("Doing api get: %s\n", api_name);
    api_interface = NULL;
    return 0;
}
