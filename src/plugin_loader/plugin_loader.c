#include "plugin_loader.h"

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <cjson.h>

#include <plugin_api.h>

// TODO: Make this better with proper stuff and the appropiate place
// TODO: Do this in compile time to get the appropriate struct then
// === STRUCT DEFINITIONS ===
#define MAX_APIS 16
#define MAX_PLUGINS 16
#define MAX_IMPLEMENTS 8
#define MAX_NAME_LEN 64
#define MAX_PATH_LEN 256

typedef struct
{
    char name[MAX_NAME_LEN];
    char plugin[MAX_NAME_LEN];
} AvailableAPI;

typedef struct
{
    char name[MAX_NAME_LEN];
    char path[MAX_PATH_LEN];
    char implements[MAX_IMPLEMENTS][MAX_NAME_LEN];
    int implements_count;
} Plugin;

typedef struct
{
    AvailableAPI available_apis[MAX_APIS];
    int available_apis_count;
    Plugin plugins[MAX_PLUGINS];
    int plugins_count;
} Config;

Config g_config;

// 1. Define a static memory arena (e.g., 4KB)
#define STATIC_BUFFER_SIZE 4096
static unsigned char static_buffer[STATIC_BUFFER_SIZE];
static size_t buffer_offset = 0;

// 2. Custom Malloc: Allocates from the static buffer
void *static_malloc(size_t size)
{
    // Ensure memory alignment (8-byte alignment is standard for most architectures)
    size_t aligned_size = (size + 7) & ~7;

    if (buffer_offset + aligned_size > STATIC_BUFFER_SIZE)
    {
        return NULL; // Out of memory
    }

    void *ptr = (void *)(static_buffer + buffer_offset);
    buffer_offset += aligned_size;
    return ptr;
}

// 3. Custom Free: No-op (we reset the entire buffer at once later)
void static_free(void *ptr)
{
    // Individual frees are not supported in a simple bump allocator.
    // Memory is reclaimed by resetting 'buffer_offset' to 0.
    (void)ptr;
}

void safe_strcpy(char *dest, const char *src, size_t dest_size)
{
    if (src == NULL)
    {
        dest[0] = '\0';
        return;
    }
    strncpy_s(dest, dest_size, src, _TRUNCATE);
}

// === PARSING FUNCTION ===
int parse_config(const char *json_str, Config *config)
{
    memset(config, 0, sizeof(Config));

    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL)
    {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return -1;
    }

    // Parse available_apis array
    cJSON *apis_array = cJSON_GetObjectItemCaseSensitive(root, "available_apis");
    if (cJSON_IsArray(apis_array))
    {
        int api_count = 0;
        cJSON *api_item = NULL;
        cJSON_ArrayForEach(api_item, apis_array)
        {
            if (api_count >= MAX_APIS)
                break;

            cJSON *name = cJSON_GetObjectItemCaseSensitive(api_item, "name");
            cJSON *plugin = cJSON_GetObjectItemCaseSensitive(api_item, "plugin");

            if (cJSON_IsString(name))
            {
                safe_strcpy(config->available_apis[api_count].name,
                            name->valuestring, MAX_NAME_LEN);
            }
            if (cJSON_IsString(plugin))
            {
                safe_strcpy(config->available_apis[api_count].plugin,
                            plugin->valuestring, MAX_NAME_LEN);
            }
            api_count++;
        }
        config->available_apis_count = api_count;
    }

    // Parse plugins array
    cJSON *plugins_array = cJSON_GetObjectItemCaseSensitive(root, "plugins");
    if (cJSON_IsArray(plugins_array))
    {
        int plugin_count = 0;
        cJSON *plugin_item = NULL;
        cJSON_ArrayForEach(plugin_item, plugins_array)
        {
            if (plugin_count >= MAX_PLUGINS)
                break;

            cJSON *name = cJSON_GetObjectItemCaseSensitive(plugin_item, "name");
            cJSON *path = cJSON_GetObjectItemCaseSensitive(plugin_item, "path");
            cJSON *implements = cJSON_GetObjectItemCaseSensitive(plugin_item, "implements");

            if (cJSON_IsString(name))
            {
                safe_strcpy(config->plugins[plugin_count].name,
                            name->valuestring, MAX_NAME_LEN);
            }
            if (cJSON_IsString(path))
            {
                safe_strcpy(config->plugins[plugin_count].path,
                            path->valuestring, MAX_PATH_LEN);
            }

            // Parse implements array
            if (cJSON_IsArray(implements))
            {
                int impl_count = 0;
                cJSON *impl_item = NULL;
                cJSON_ArrayForEach(impl_item, implements)
                {
                    if (impl_count >= MAX_IMPLEMENTS)
                        break;
                    if (cJSON_IsString(impl_item))
                    {
                        safe_strcpy(config->plugins[plugin_count].implements[impl_count],
                                    impl_item->valuestring, MAX_NAME_LEN);
                        impl_count++;
                    }
                }
                config->plugins[plugin_count].implements_count = impl_count;
            }
            plugin_count++;
        }
        config->plugins_count = plugin_count;
    }

    cJSON_Delete(root);
    return 0;
}

int32_t plugin_api_init()
{
    FILE *system_json_file;
    errno_t ret = fopen_s(&system_json_file, "../system.json", "rb");
    (void)ret;

    // TODO: Do this without malloc (get a define with the size of the file)
    fseek(system_json_file, 0, SEEK_END);
    size_t length = ftell(system_json_file);
    fseek(system_json_file, 0, SEEK_SET);
    // Reserve 1 byte for the null terminator
    char *buffer = malloc(length + 1);
    if (buffer)
    {
        fread(buffer, 1, length, system_json_file);
        buffer[length] = '\0';
    }
    fclose(system_json_file);

    if (parse_config(buffer, &g_config) != 0)
    {
        return 1;
    }

    // Print parsed data
    printf("=== Available APIs ===\n");
    for (int i = 0; i < g_config.available_apis_count; i++)
    {
        printf("  [%d] Name: %s\n      Plugin: %s\n",
               i,
               g_config.available_apis[i].name,
               g_config.available_apis[i].plugin);
    }

    printf("\n=== Plugins ===\n");
    for (int i = 0; i < g_config.plugins_count; i++)
    {
        printf("  [%d] Name: %s\n", i, g_config.plugins[i].name);
        printf("      Path: %s\n", g_config.plugins[i].path);
        printf("      Implements: ");
        for (int j = 0; j < g_config.plugins[i].implements_count; j++)
        {
            printf("%s%s", g_config.plugins[i].implements[j],
                   (j < g_config.plugins[i].implements_count - 1) ? ", " : "");
        }
        printf("\n");
    }

    buffer_offset = 0; // Reset allocator

    return 0;
}

int32_t plugin_api_get(const char *api_name, const void **out_plugin_interface)
{
    (void *)api_name;
    printf("Doing api get: %s\n", api_name);
    out_plugin_interface = NULL;
    return 0;
}

// TODO: Check why implements is needed!!!
int32_t plugin_api_add(const char *api_name, const void **out_plugin_interface)
{
    char *real_plugin_name = NULL;

    for (int i = 0; i < g_config.available_apis_count; i++)
    {
        AvailableAPI *available_api = &g_config.available_apis[i];
        if (strcmp(available_api->name, api_name) == 0)
        {
            real_plugin_name = available_api->plugin;
        }
    }

    if (real_plugin_name == NULL)
    {
        printf("Plugin \"%s\" not found\n", api_name);
        return -1;
    }

    printf("Found plugin! Name: %s\n", real_plugin_name);
    for (int i = 0; i < g_config.plugins_count; i++)
    {
        Plugin *plugin = &g_config.plugins[i];
        if (strcmp(plugin->name, real_plugin_name) == 0)
        {
            printf("plugin: %s - lib path: %s\n", plugin->name, plugin->path);
            // TODO: Load library here
            // TODO: Make sure to call appropriate methods here from the library
        }
        // for (int j = 0; j < plugin->implements_count; j++)
        // {
        // printf("%s == %s: %d\n", plugin->implements[j], real_plugin_name, strcmp(plugin->implements[j], real_plugin_name));
        // if (strcmp(plugin->implements[j], real_plugin_name) == 0)
        // {
        //     printf("plugin: %s - lib path: %s\n", plugin->name, plugin->path);
        // }
        // }
    }

    *out_plugin_interface = NULL;
    return -1;
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
