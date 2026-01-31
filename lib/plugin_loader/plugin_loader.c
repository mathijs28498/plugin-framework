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

#define PLUGIN_LOADER_MAX_PLUGIN_COUNT 64

typedef struct
{
    char api_name[PLUGIN_CONFIG_MAX_PLUGIN_API_NAME_COUNT];
    char plugin_name[PLUGIN_CONFIG_MAX_PLUGIN_NAME_COUNT];
} PluginToAddInfo;

size_t g_plugins_to_add_count = 0;
PluginToAddInfo g_plugins_to_add[PLUGIN_LOADER_MAX_PLUGIN_COUNT];

int32_t plugin_loader_read_config(char **buffer_out)
{
    FILE *system_json_file;
    int ret;
    ret = fopen_s(&system_json_file, "../plugin_config.json", "rb");

    // TODO: Do this without malloc (get a define with the size of the file)
    fseek(system_json_file, 0, SEEK_END);
    size_t length = ftell(system_json_file);
    fseek(system_json_file, 0, SEEK_SET);
    // Reserve 1 byte for the null terminator
    *buffer_out = malloc(length + 1);
    if (*buffer_out)
    {
        fread(*buffer_out, 1, length, system_json_file);
        (*buffer_out)[length] = '\0';
    }
    fclose(system_json_file);

    return 0;
}

int32_t plugin_api_init(void)
{
    int ret;
    char *buffer;
    PluginConfig plugin_config;

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
    //          - Load in plugins to add
    //          - Call dependency functions on plugins to add
    //          - Check if dependencies are or are not included
    //          - Add dependencies where not already included
    //          - Repeat this until all dependencies loaded in
    //          - Create dependency graph
    //              - Do error checking on these dependencies (eg. cyclic dependencies)
    //          - Walk dependency graph layer by layer calling the init functions

    // TODO: Check if this algorithm can/should be made better/faster
    // Get the PluginInfo for the dlls of the user added
    for (size_t i = 0; i < g_plugins_to_add_count; i++)
    {
        PluginToAddInfo *plugin_to_add = &g_plugins_to_add[i];

        bool use_default = strlen(plugin_to_add->plugin_name) == 0;

        int32_t plugin_info_index = -1;
        for (uint32_t j = 0; j < plugin_config.plugins_count; j++)
        {
            PluginInfo *plugin_info = &plugin_config.plugins[j];

            if (use_default)
            {
                if (strncmp(plugin_info->implements, plugin_to_add->api_name, PLUGIN_CONFIG_MAX_PLUGIN_API_NAME_COUNT) == 0)
                {
                    plugin_info_index = (int32_t)j;
                    break;
                }
            }
            else
            {
                if (strncmp(plugin_info->name, plugin_to_add->plugin_name, PLUGIN_CONFIG_MAX_PLUGIN_NAME_COUNT) == 0)
                {
                    plugin_info_index = (int32_t)j;
                    break;
                }
            }
        }

        if (plugin_info_index < 0)
        {
            printf("Error: couldn't add plugin api \"%s\"\n", plugin_to_add->api_name);
            return -1;
        }

        printf("Adding plugin api: \"%s\" with implementation \"%s\" and path \"%s\"\n", 
            plugin_config.plugins[plugin_info_index].implements,
            plugin_config.plugins[plugin_info_index].name,
            plugin_config.plugins[plugin_info_index].path
        );

        // for (int )
    }

    printf("\n=== Plugins ===\n");
    for (uint32_t i = 0; i < plugin_config.plugins_count; i++)
    {
        printf("\n  [%d] Name: %s\n", i, plugin_config.plugins[i].name);
        printf("      Path: %s\n", plugin_config.plugins[i].path);
        printf("      Implements: %s\n", plugin_config.plugins[i].implements);
        HMODULE loaded_dll = LoadLibrary(plugin_config.plugins[i].path);
        if (!loaded_dll)
        {
            printf("Error: Failed to load plugin \"%s\" at \"%s\"\n", plugin_config.plugins[i].name, plugin_config.plugins[i].path);
            // return -1;
            continue;
        }

        FARPROC proc = GetProcAddress(loaded_dll, "test_api_get_dependencies");
        // FARPROC proc = GetProcAddress(loaded_dll, "test_func");
        // FARPROC proc = GetProcAddress(loaded_dll, "test_func_");
        if (!proc)
        {
            printf("Error: Plugin \"%s\" does not have a get_dependencies function defined\n", plugin_config.plugins[i].name);
            continue;
        }

        printf("Calling proc\n");
        const char *const *dependencies;
        int32_t count;

        proc(&dependencies, &count);
        for (int j = 0; j < count; j++)
        {
            printf("dep: %s\n", dependencies[j]);
        }

        FARPROC proc2 = GetProcAddress(loaded_dll, "set_dependency_test_api_2");
        if (!proc2)
        {
            printf("Error: Plugin \"%s\" does not have a set_dependency_test_api_2 function defined\n", plugin_config.plugins[i].name);
            continue;
        }
        proc2(NULL);
    }
    return 0;
}

int32_t plugin_api_get(const char *api_name, const void **out_api_interface)
{
    (void *)api_name;
    printf("Doing api get: %s\n", api_name);
    out_api_interface = NULL;
    return 0;
}

// // TODO: Check why implements is needed!!!
int32_t plugin_api_add(const char *api_name, const char *plugin_name)
{
    if (api_name == NULL)
    {
        printf("Error - Api name is NULL\n");
        return -1;
    }
    strncpy_s(g_plugins_to_add[g_plugins_to_add_count].api_name, PLUGIN_CONFIG_MAX_PLUGIN_API_NAME_COUNT, api_name, strlen(api_name));

    if (plugin_name == NULL)
    {
        g_plugins_to_add[g_plugins_to_add_count].plugin_name[0] = '\0';
    }
    else
    {
        strncpy_s(g_plugins_to_add[g_plugins_to_add_count].plugin_name, PLUGIN_CONFIG_MAX_PLUGIN_NAME_COUNT, plugin_name, strlen(plugin_name));
    }

    g_plugins_to_add_count++;
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
    //     for (int i = 0; i < g_config.plugins_count; i++)
    //     {
    //         Plugin *plugin = &g_config.plugins[i];
    //         if (strcmp(plugin->name, real_plugin_name) == 0)
    //         {
    //             printf("plugin: %s - lib path: %s\n", plugin->name, plugin->path);
    //             // TODO: Load library here
    //             // TODO: Make sure to call appropriate methods here from the library
    //         }
    //         // for (int j = 0; j < plugin->implements_count; j++)
    //         // {
    //         // printf("%s == %s: %d\n", plugin->implements[j], real_plugin_name, strcmp(plugin->implements[j], real_plugin_name));
    //         // if (strcmp(plugin->implements[j], real_plugin_name) == 0)
    //         // {
    //         //     printf("plugin: %s - lib path: %s\n", plugin->name, plugin->path);
    //         // }
    //         // }
    //     }
};

PluginApi g_plugin_api = {
    .init = plugin_api_init,
    .add = plugin_api_add,
    .get = plugin_api_get,
};

PluginApi *get_plugin_api()
{
    return &g_plugin_api;
}
