#include "plugin_manager.h"

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <Windows.h>
#include <assert.h>

#include <plugin_framework.h>
#include <plugin_utils.h>

#include <environment_interface.h>
#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(plugin_manager, LOG_LEVEL_DEBUG)

#include "plugin_manager_interface_declarations.h"
#include "plugin_registry.h"
#include "plugin_manager_types.h"
#include "plugin_manager_loader.h"
#include "plugin_manager_init_contexts.h"

// #if PLUGIN_BUILD_SHARED
#define PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH 256

int32_t plugin_manager_load(PluginManagerSetupContext *setup_context, PluginManagerRuntimeContext *runtime_context)
{
    int ret;
    LoggerInterface *logger = setup_context->logger;
    runtime_context->logger = logger;

    const PluginRegistry *plugin_registry = plugin_registry_get();

    SAFE_WHILE(
        setup_context->requested_plugins_len > 0,
        PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH,
        {
            LOG_ERR("Plugin manager dependency solver exceeded max interations");
            return -1;
        })
    {

        {
            PluginModule plugin_modules[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
            size_t plugin_modules_len = 0;

            ret = resolve_requested_plugins(
                logger,
                setup_context->requested_plugins,
                setup_context->requested_plugins_len,
                plugin_registry,
                setup_context->plugin_providers,
                setup_context->plugin_providers_len,
                plugin_modules,
                &plugin_modules_len);

            if (ret < 0)
            {
                LOG_ERR("Error in resolve_requested_plugins: %d", ret);
                return ret;
            }

            ret = load_plugin_modules(
                logger,
                plugin_modules,
                plugin_modules_len,
                setup_context->plugin_providers,
                &setup_context->plugin_providers_len);

            if (ret < 0)
            {
                LOG_ERR("Error in load_plugin_modules: %d", ret);
                return ret;
            }
        }

        setup_context->requested_plugins_len = 0;

        ret = resolve_plugin_provider_dependencies(
            requested_plugins, &requested_plugins_len);
            logger,
            true,
            setup_context->plugin_providers,
            setup_context->plugin_providers_len,
            setup_context->requested_plugins,
            &setup_context->requested_plugins_len);

        if (ret < 0)
        {
            LOG_ERR("Error in resolve_plugin_provider_dependencies: %d", ret);
            return ret;
        }
    }

    LOG_DBG("plugin_providers_len %d", setup_context->plugin_providers_len);

    ret = calculate_plugin_provider_initialization_order(
        logger,
        setup_context->plugin_providers,
        setup_context->plugin_providers_len,
        setup_context->sorted_plugin_providers_indices);
    if (ret < 0)
    {
        LOG_ERR("Error in calculate_plugin_provider_initialization_order: %d", ret);
        return ret;
    }

    ret = initialize_plugins(
        logger,
        setup_context->sorted_plugin_providers_indices,
        setup_context->plugin_providers,
        setup_context->plugin_providers_len,
        runtime_context->interface_instances,
        &runtime_context->interface_instances_len);
    if (ret < 0)
    {
        LOG_ERR("Error in initialize_plugins: %d", ret);
        return ret;
    }

    return 0;
}

int32_t plugin_manager_request_plugin(
    const LoggerInterface *logger,
    const char *interface_name,
    const char *plugin_name,
    bool is_explicit,
    RequestedPlugin *requested_plugins,
    size_t *requested_plugins_len)
{
    if (interface_name == NULL)
    {
        LOG_ERR("Interface name is NULL");
        return -1;
    }

    if (*requested_plugins_len >= PLUGIN_MANAGER_MAX_PLUGINS_LEN)
    {
        LOG_ERR("Cannot add plugin as max plugin_definitions is reached. Max plugin count '%d'", PLUGIN_MANAGER_MAX_PLUGINS_LEN);
        return -1;
    }

    struct RequestedPlugin *requested_plugin = &requested_plugins[*requested_plugins_len];
    requested_plugin->interface_name = interface_name;

    if (plugin_name == NULL)
    {
        requested_plugin->plugin_name = "";
    }
    else
    {
        requested_plugin->plugin_name = plugin_name;
    }

    requested_plugin->is_explicit = is_explicit;
    requested_plugin->is_resolved = false;

    (*requested_plugins_len)++;
    return 0;
}
// #endif // #if PLUGIN_BUILD_SHARED

int32_t __plugin_manager_init(int argc, char **argv, void *platform_context, PluginManagerSetupContext **setup_context, PluginManagerRuntimeContext **runtime_context)
{
    assert(setup_context != NULL);
    assert(runtime_context != NULL);

    int ret;
    LoggerInterface *logger;

    ret = plugin_manager_init_contexts(setup_context, runtime_context);
    if (ret < 0)
    {
        if (*setup_context != NULL && (*setup_context)->logger)
        {
            logger = (*setup_context)->logger;
            LOG_ERR("Error getting setup context: %d", ret);
        }
        return ret;
    }

    logger = (*setup_context)->logger;

    environment_default_set_args((*setup_context)->environment->context, argc, argv, platform_context);

#if PLUGIN_BUILD_SHARED
    ret = plugin_manager_load(*setup_context, *runtime_context);
    if (ret < 0)
    {
        LOG_ERR("Error loading plugins: %d", ret);
        return ret;
    }
#endif // #if PLUGIN_BUILD_SHARED

    return 0;
}

int32_t __plugin_manager_get(PluginManagerRuntimeContext *runtime_context, const char *interface_name, void **iface)
{
    LoggerInterface *logger = runtime_context->logger;
    for (size_t i = 0; i < runtime_context->interface_instances_len; i++)
    {
        InterfaceInstance *interface_instance = &runtime_context->interface_instances[i];
        if (strcmp(interface_name, interface_instance->interface_name) == 0)
        {
            if (!interface_instance->is_explicit)
            {
                LOG_ERR("Interface '%s' found but is not added explicitly. Consider adding it explicitly", interface_name);
                *iface = NULL;
                return -1;
            }
            *iface = interface_instance->iface;
            return 0;
        }
    }

    *iface = NULL;
    LOG_ERR("Failed to get interface '%s'", interface_name);

    return -1;
}

int32_t __plugin_manager_shutdown(PluginManagerSetupContext *setup_context, PluginManagerRuntimeContext *runtime_context, int exit_code)
{
    LoggerInterface *logger = setup_context->logger;
    for (int i = (int)setup_context->plugin_providers_len - 1; i >= 0; i--)
    {
        size_t current_idx = setup_context->sorted_plugin_providers_indices[i];
        PluginProvider *plugin_provider = &setup_context->plugin_providers[current_idx];
        if (!plugin_provider->is_initialized)
        {
            LOG_DBG("Skipping shutdown '%s' interface as interface is not initialized", plugin_provider->interface_name);
            continue;
        }

        InterfaceInstance *interface_instance = &runtime_context->interface_instances[i];
        LOG_DBG("Shutting down '%s' interface", plugin_provider->interface_name);

        if (plugin_provider->shutdown)
        {
            plugin_provider->shutdown(interface_instance->iface->context);
        }
    }

    (void)logger_console_on_program_exit(setup_context->logger->context, exit_code);

    return 0;
}