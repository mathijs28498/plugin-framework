#include "plugin_manager.h"

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <cjson.h>
#include <Windows.h>
#include <assert.h>

#include <plugin_framework.h>

#include <environment_interface.h>
#include <environment_default.h>
#include <environment_default_register.h>
#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(plugin_manager, LOG_LEVEL_DEBUG)
#include <logger_console.h>
#include <logger_console_register.h>

#include "file_io.h"
#include "plugin_registry.h"
#include "plugin_manager_types.h"
#include "plugin_manager_loader.h"

#define PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH 256

int32_t __plugin_manager_init(int argc, char **argv, void *platform_context, PluginManagerSetupContext **setup_context, PluginManagerRuntimeContext **runtime_context)
{
    assert(setup_context != NULL);

    static PluginManagerSetupContext new_setup_context = {
        .internal_plugins_len = 0,
        .plugin_providers_len = 0,
        .requested_plugins_len = 0,
    };

    static PluginManagerRuntimeContext new_runtime_context = {0};

    {
        assert(new_setup_context.internal_plugins_len < ARRAY_SIZE(new_setup_context.internal_plugins));

        EnvironmentInterface *environment = environment_get_interface();
        environment_default_set_args(environment->context, argc, argv, platform_context);

        PluginProvider environment_plugin = {
            .interface_name = "environment",
            .plugin_name = "default",
            .dependencies_len = 0,
            .get_interface = (PluginGetInterface_Fn)environment_get_interface,
            .init = NULL,
            .shutdown = NULL,
            .is_initialized = true,
        };

        memcpy(&new_setup_context.internal_plugins[new_setup_context.internal_plugins_len],
               &environment_plugin,
               sizeof(new_setup_context.internal_plugins[new_setup_context.internal_plugins_len]));
        new_setup_context.internal_plugins_len++;
    }

    {
        assert(new_setup_context.internal_plugins_len < ARRAY_SIZE(new_setup_context.internal_plugins));

        TODO("Add init function so that the allocConsole gets done properly")
        PluginProvider logger_plugin = {
            .interface_name = "logger",
            .plugin_name = "console",
            .dependencies_len = 0,
            .get_interface = (PluginGetInterface_Fn)logger_get_interface,
            .init = NULL,
            .shutdown = NULL,
            .is_initialized = true,
        };

        new_setup_context.logger = (LoggerInterface *)logger_plugin.get_interface();
        new_runtime_context.logger = (LoggerInterface *)logger_plugin.get_interface();

        memcpy(&new_setup_context.internal_plugins[new_setup_context.internal_plugins_len],
               &logger_plugin,
               sizeof(new_setup_context.internal_plugins[new_setup_context.internal_plugins_len]));
        new_setup_context.internal_plugins_len++;
    }

    *setup_context = &new_setup_context;
    *runtime_context = &new_runtime_context;
    return 0;
}

int32_t __plugin_manager_add(PluginManagerSetupContext *setup_context, const char *interface_name, const char *plugin_name)
{
    return plugin_manager_add_internal(
        setup_context->logger,
        interface_name,
        plugin_name,
        true,
        setup_context->requested_plugins,
        &setup_context->requested_plugins_len);
};

int32_t plugin_manager_add_internal(
    const LoggerInterface *logger,
    const char *interface_name,
    const char *plugin_name,
    bool is_explicit,
    RequestedPlugin *requested_plugins,
    size_t *requested_plugins_len)
{
    if (interface_name == NULL)
    {
        LOG_ERR(logger, "Interface name is NULL");
        return -1;
    }

    if (*requested_plugins_len >= PLUGIN_MANAGER_MAX_PLUGINS_LEN)
    {
        LOG_ERR(logger, "Cannot add plugin as max plugin_definitions is reached. Max plugin count '%d'", PLUGIN_MANAGER_MAX_PLUGINS_LEN);
        return -1;
    }

    struct RequestedPlugin *requested_plugin = &requested_plugins[*requested_plugins_len];

    snprintf(requested_plugin->interface_name, PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN, "%s", interface_name);

    if (plugin_name == NULL)
    {
        requested_plugin->plugin_name[0] = '\0';
    }
    else
    {
        snprintf(requested_plugin->plugin_name, PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN, "%s", plugin_name);
    }

    requested_plugin->is_explicit = is_explicit;
    requested_plugin->is_resolved = false;

    (*requested_plugins_len)++;
    return 0;
}

int32_t __plugin_manager_load(PluginManagerSetupContext *setup_context, PluginManagerRuntimeContext *runtime_context)
{
    int ret;
    char *buffer;
    PluginModuleRegistry plugin_registry;
    LoggerInterface *logger = setup_context->logger;
    runtime_context->logger = logger;

    TODO("Use arena allocation for this")
    ret = file_io_read(logger, "../plugin_registry.json", &buffer);
    ret = plugin_registry_deserialize_json(logger, buffer, &plugin_registry);
    free(buffer);

    if (ret < 0)
    {
        LOG_ERR(logger, "unable to parse plugin config: %d", ret);
        return ret;
    }

    SAFE_WHILE(
        setup_context->requested_plugins_len > 0,
        PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH,
        {
            LOG_ERR(logger, "Plugin manager dependency solver exceeded max interations");
            return -1;
        })
    {

        {
            PluginModule plugin_modules[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
            size_t plugin_modules_len = 0;

            ret = resolve_requested_plugins_registry(
                logger,
                setup_context->requested_plugins,
                setup_context->requested_plugins_len,
                &plugin_registry,
                plugin_modules,
                &plugin_modules_len);

            if (ret < 0)
            {
                LOG_ERR(logger, "Error in resolve_requested_plugins_registry: %d", ret);
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
                LOG_ERR(logger, "Error in load_plugin_modules: %d", ret);
                return ret;
            }
        }

        ret = resolve_requested_plugins_internal(
            logger,
            setup_context->requested_plugins,
            setup_context->requested_plugins_len,
            setup_context->internal_plugins,
            setup_context->internal_plugins_len,
            setup_context->plugin_providers,
            &setup_context->plugin_providers_len);

        if (ret < 0)
        {
            LOG_ERR(logger, "Error in resolve_requested_plugins_internal: %d", ret);
            return ret;
        }

        setup_context->requested_plugins_len = 0;

        ret = resolve_plugin_provider_dependencies(
            logger,
            setup_context->plugin_providers,
            setup_context->plugin_providers_len,
            setup_context->requested_plugins,
            &setup_context->requested_plugins_len);

        if (ret < 0)
        {
            LOG_ERR(logger, "Error in resolve_plugin_provider_dependencies: %d", ret);
            return ret;
        }
    }

    ret = calculate_plugin_provider_initialization_order(logger, setup_context->plugin_providers, setup_context->plugin_providers_len, setup_context->sorted_plugin_providers_indices);
    if (ret < 0)
    {
        LOG_ERR(logger, "Error in calculate_plugin_provider_initialization_order: %d", ret);
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
        LOG_ERR(logger, "Error in initialize_plugins: %d", ret);
        return ret;
    }

    return 0;
}

int32_t __plugin_manager_get(PluginManagerRuntimeContext *runtime_context, const char *interface_name, void **iface)
{
    for (size_t i = 0; i < runtime_context->interface_instances_len; i++)
    {
        InterfaceInstance *interface_instance = &runtime_context->interface_instances[i];
        if (strcmp(interface_name, interface_instance->interface_name) == 0 && interface_instance->is_explicit)
        {
            *iface = interface_instance->iface;
            return 0;
        }
    }

    *iface = NULL;
    LOG_ERR(runtime_context->logger, "Failed to get interface '%s'", interface_name);

    return -1;
}

int32_t __plugin_manager_shutdown(PluginManagerSetupContext *setup_context, PluginManagerRuntimeContext *runtime_context, int exit_code)
{
    for (int i = (int)setup_context->plugin_providers_len - 1; i >= 0; i--)
    {
        size_t current_idx = setup_context->sorted_plugin_providers_indices[i];
        PluginProvider *plugin_provider = &setup_context->plugin_providers[current_idx];
        InterfaceInstance *interface_instance = &runtime_context->interface_instances[i];
        LOG_DBG(setup_context->logger, "Shutting down '%s' interface", plugin_provider->interface_name);

        if (plugin_provider->shutdown)
        {
            plugin_provider->shutdown(interface_instance->iface->context);
        }
    }

    (void)logger_console_on_program_exit(setup_context->logger->context, exit_code);

    return 0;
}