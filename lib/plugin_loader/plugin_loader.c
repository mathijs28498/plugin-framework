#include "plugin_loader.h"

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <cjson.h>
#include <Windows.h>

#include <plugin_api.h>

#include "plugin_loader_json_parser.h"
#include "plugin_loader_config_reader.h"

// TODO: Remove global variables for local ones

#define PLUGIN_LOADER_MAX_PLUGIN_LEN 64

typedef struct
{
    char api_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN];
    char plugin_name[PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN];
} RequestedPlugin;

// size_t plugins_to_add_count = 0;
// RequestedPlugin requested_plugins[PLUGIN_LOADER_MAX_PLUGIN_LEN];

typedef struct
{
    PluginDefinition *plugin_definition;

    HMODULE dll;
    char **dependencies;
    uint32_t dependencies_len;
    void *api;
} PluginModule;

struct PluginLoaderContext
{
    size_t requested_plugins_len;
    RequestedPlugin requested_plugins[PLUGIN_LOADER_MAX_PLUGIN_LEN];

    // TODO: See if this can be only known in the load function, not context
    // size_t plugin_modules_len;
    // PluginModule plugin_modules[PLUGIN_LOADER_MAX_PLUGIN_LEN];
};

// size_t g_plugin_data_count = 0;
// PluginModule g_plugin_data[PLUGIN_LOADER_MAX_PLUGIN_LEN];

int32_t resolve_requested_plugins(PluginLoaderContext *context, PluginModule *plugin_modules, size_t *plugin_modules_len, PluginRegistry *plugin_registry)
{
    for (size_t i = 0; i < context->requested_plugins_len; i++)
    {
        RequestedPlugin *requested_plugin = &context->requested_plugins[i];

        bool use_default = strlen(requested_plugin->plugin_name) == 0;

        int32_t plugin_info_index = -1;
        for (uint32_t j = 0; j < plugin_registry->plugin_definitions_len; j++)
        {
            PluginDefinition *plugin_definition = &plugin_registry->plugin_definitions[j];

            char *plugin_info_name = use_default
                                         ? plugin_definition->api
                                         : plugin_definition->name;

            char *plugin_to_add_name = use_default
                                           ? requested_plugin->api_name
                                           : requested_plugin->plugin_name;

            if (strcmp(plugin_info_name, plugin_to_add_name) == 0)
            {
                plugin_info_index = (int32_t)j;
                break;
            }
        }

        if (plugin_info_index < 0)
        {
            printf("Error: couldn't add plugin api \"%s\"\n", requested_plugin->api_name);
            return -1;
        }
        plugin_modules[*plugin_modules_len].plugin_definition = &plugin_registry->plugin_definitions[plugin_info_index];
        (*plugin_modules_len)++;
        // TODO: Add max index check here
    }
    
    return 0;
}

int32_t plugin_api_load(PluginLoaderContext *context)
{
    int ret;
    char *buffer;
    PluginRegistry plugin_config;

    // TODO: Make buffer not use malloc
    ret = plugin_loader_read_config(&buffer);
    ret = plugin_loader_parse_config(buffer, &plugin_config);
    free(buffer);

    if (ret < 0)
    {
        printf("Error - unable to parse plugin config: %d", ret);
        return ret;
    }

    // TODO: plugin add functionality
    //          [X] - Load in plugin_definitions to add
    //          [X] - Call dependency functions on plugin_definitions to add
    //          [ ] - Check if dependencies are or are not included
    //          [ ] - Add dependencies where not already included
    //          [ ] - Repeat this until all dependencies loaded in
    //          [ ] - Create dependency graph
    //              [ ] - Do error checking on these dependencies (eg. cyclic dependencies)
    //          [/] - Walk dependency graph layer by layer calling the init functions

    // TODO: Check if this algorithm can/should be made better/faster
    size_t plugin_modules_len = 0;
    PluginModule plugin_modules[PLUGIN_LOADER_MAX_PLUGIN_LEN];
    resolve_requested_plugins(context, plugin_modules, &plugin_modules_len, &plugin_config);

    // TODO: Make own method - get dependencies
    for (size_t i = 0; i < plugin_modules_len; i++)
    {
        PluginModule *plugin_module = &plugin_modules[i];

        plugin_module->dll = LoadLibrary(plugin_module->plugin_definition->path);
        if (!plugin_module->dll)
        {
            printf("Error: Failed to load plugin \"%s\" at \"%s\"\n", plugin_module->plugin_definition->name, plugin_module->plugin_definition->path);
            continue;
        }

        // TODO: Check where this belongs, should be 0 if no dependencies call is found or maybe always init to 0 for the whole buffer?
        plugin_module->dependencies_len = 0;
        static const char get_dependencies_postfix[] = "_get_dependencies";
        char get_dependencies_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(get_dependencies_postfix)];

        snprintf(get_dependencies_function_name, sizeof(get_dependencies_function_name),
                 "%s%s", plugin_module->plugin_definition->api, get_dependencies_postfix);

        FARPROC get_dependencies_proc = GetProcAddress(plugin_module->dll, get_dependencies_function_name);
        if (get_dependencies_proc)
        {
            // TODO: Make these functions casted to a function signature typedef
            get_dependencies_proc(&plugin_module->dependencies, &plugin_module->dependencies_len);
        }

        static const char get_api_postfix[] = "_get_api";
        char get_api_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(get_api_postfix)];

        snprintf(get_api_function_name, sizeof(get_api_function_name),
                 "%s%s", plugin_module->plugin_definition->api, get_api_postfix);

        FARPROC get_api_proc = GetProcAddress(plugin_module->dll, get_api_function_name);
        if (get_api_proc)
        {
            // TODO: Make these functions casted to a function signature typedef
            plugin_module->api = (void *)get_api_proc();
        }
        else
        {
            printf("Error: no api found for plugin: %s\n", plugin_module->plugin_definition->name);
        }
    }

    // TODO: Make own method - resolve dependencies
    // TODO: Make sure the plugin_definitions get initialized in order with their dependencies
    for (size_t i = 0; i < plugin_modules_len; i++)
    {
        PluginModule *plugin_module = &plugin_modules[i];

        for (uint32_t j = 0; j < plugin_module->dependencies_len; j++)
        {
            // TODO: Handle when dependency is not found (add it from the registry if it is available)
            char *dependency = plugin_module->dependencies[j];
            for (uint32_t k = 0; k < plugin_modules_len; k++)
            {
                PluginModule *dep_plugin_module = &plugin_modules[k];
                if (strcmp(dep_plugin_module->plugin_definition->api, dependency) == 0)
                {

                    static const char set_dependency_midfix[] = "_set_";
                    // TODO: Fix the sizing here
                    char set_dependency_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(set_dependency_midfix)];

                    snprintf(set_dependency_function_name, sizeof(set_dependency_function_name),
                             "%s%s%s", plugin_module->plugin_definition->api, set_dependency_midfix, dependency);

                    FARPROC set_dependency_proc = GetProcAddress(plugin_module->dll, set_dependency_function_name);
                    if (set_dependency_proc)
                    {
                        // TODO: Make these functions casted to a function signature typedef
                        set_dependency_proc(dep_plugin_module->api);
                    }
                    else
                    {
                        printf("Error: %s() not found\n", set_dependency_function_name);
                    }
                }
            }
        }

        static const char init_postfix[] = "_init";
        // TODO: Fix the sizing here
        char init_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(init_postfix)];
        snprintf(init_function_name, sizeof(init_function_name),
                 "%s%s", plugin_module->plugin_definition->api, init_postfix);

        FARPROC init_proc = GetProcAddress(plugin_module->dll, init_function_name);
        if (init_proc)
        {
            // TODO: Make these functions casted to a function signature typedef
            int32_t init_ret = (int32_t)init_proc();
            // TODO: Do something with the return value;
            (void)init_ret;
        }
        else
        {
            printf("Error: %s() not found\n", init_function_name);
        }
    }

    // printf("\n=== Plugins ===\n");
    // for (uint32_t i = 0; i < plugin_config.plugin_definitions_len; i++)
    // {
    //     printf("\n  [%d] Name: %s\n", i, plugin_config.plugin_definitions[i].name);
    //     printf("      Path: %s\n", plugin_config.plugin_definitions[i].path);
    //     printf("      Implements: %s\n", plugin_config.plugin_definitions[i].api);
    //     HMODULE loaded_dll = LoadLibrary(plugin_config.plugin_definitions[i].path);
    //     if (!loaded_dll)
    //     {
    //         printf("Error: Failed to load plugin \"%s\" at \"%s\"\n", plugin_config.plugin_definitions[i].name, plugin_config.plugin_definitions[i].path);
    //         // return -1;
    //         continue;
    //     }

    //     FARPROC proc = GetProcAddress(loaded_dll, "test_api_get_dependencies");
    //     // FARPROC proc = GetProcAddress(loaded_dll, "test_func");
    //     // FARPROC proc = GetProcAddress(loaded_dll, "test_func_");
    //     if (!proc)
    //     {
    //         printf("Error: Plugin \"%s\" does not have a get_dependencies function defined\n", plugin_config.plugin_definitions[i].name);
    //         continue;
    //     }

    //     printf("Calling proc\n");
    //     const char *const *dependencies;
    //     int32_t count;

    //     proc(&dependencies, &count);
    //     for (int j = 0; j < count; j++)
    //     {
    //         printf("dep: %s\n", dependencies[j]);
    //     }

    //     FARPROC proc2 = GetProcAddress(loaded_dll, "set_dependency_test_api_2");
    //     if (!proc2)
    //     {
    //         printf("Error: Plugin \"%s\" does not have a set_dependency_test_api_2 function defined\n", plugin_config.plugin_definitions[i].name);
    //         continue;
    //     }
    //     proc2(NULL);
    // }
    return 0;
}

int32_t plugin_api_get(const PluginLoaderContext *context, const char *api_name, const void **out_api_interface)
{
    (void)context;
    (void)api_name;
    printf("Doing api get: %s\n", api_name);
    out_api_interface = NULL;
    return 0;
}

int32_t plugin_api_add(PluginLoaderContext *context, const char *api_name, const char *plugin_name)
{
    if (api_name == NULL)
    {
        printf("Error - Api name is NULL\n");
        return -1;
    }
    if (context->requested_plugins_len >= PLUGIN_LOADER_MAX_PLUGIN_LEN)
    {
        printf("Error - Cannot add plugin as max plugin_definitions is reached. Max plugin count \"%d\"\n", PLUGIN_LOADER_MAX_PLUGIN_LEN);
        return -1;
    }

    // TODO:
    snprintf(context->requested_plugins[context->requested_plugins_len].api_name, PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN,
             "%s", api_name);

    // strncpy_s(context
    //     requested_plugins[plugins_to_add_count].api_name, PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN, api_name, strlen(api_name));

    if (plugin_name == NULL)
    {
        context->requested_plugins[context->requested_plugins_len].plugin_name[0] = '\0';
    }
    else
    {
        snprintf(context->requested_plugins[context->requested_plugins_len].plugin_name, PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN,
                 "%s", plugin_name);
    }

    context->requested_plugins_len++;
    return 0;
    //     char *real_plugin_name = NULL;

    //     for (int i = 0; i < g_config.available_apis_count; i++)
    //     {
    //         AvailableAPI *available_api = &g_config.available_apis[i];
    //         if (strcmp(available_api->name, api_name) == 0)
    //         {
    //             real_plugin_name = available_api->plugin;
    //         }
    //     }

    //     if (real_plugin_name == NULL)
    //     {
    //         printf("Plugin \"%s\" not found\n", api_name);
    //         return -1;
    //     }

    //     printf("Found plugin! Name: %s\n", real_plugin_name);
    //     for (int i = 0; i < g_config.plugin_definitions_len; i++)
    //     {
    //         Plugin *plugin = &g_config.plugin_definitions[i];
    //         if (strcmp(plugin->name, real_plugin_name) == 0)
    //         {
    //             printf("plugin: %s - lib path: %s\n", plugin->name, plugin->path);
    //             // TODO: Load library here
    //             // TODO: Make sure to call appropriate methods here from the library
    //         }
    //         // for (int j = 0; j < plugin->implements_count; j++)
    //         // {
    //         // printf("%s == %s: %d\n", plugin->api[j], real_plugin_name, strcmp(plugin->api[j], real_plugin_name));
    //         // if (strcmp(plugin->api[j], real_plugin_name) == 0)
    //         // {
    //         //     printf("plugin: %s - lib path: %s\n", plugin->name, plugin->path);
    //         // }
    //         // }
    //     }
};

PluginApi *get_plugin_api()
{
    static PluginLoaderContext context = {
        .requested_plugins_len = 0,
    };

    static PluginApi plugin_api = {
        .context = &context,

        .add = plugin_api_add,
        .load = plugin_api_load,
        .get = plugin_api_get,
    };

    return &plugin_api;
}
