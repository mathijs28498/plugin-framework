#include "plugin_registry.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>
#include <stdio.h>

#include <plugin_manager_common.h>

#include "plugin_manager_types.h"

int32_t plugin_registry_deserialize_json(const char *json_str, PluginRegistry *plugin_registry)
{
    memset(plugin_registry, 0, sizeof(PluginRegistry));

    cJSON *json_root = cJSON_Parse(json_str);
    if (json_root == NULL)
    {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return -1;
    }

    const cJSON *json_plugin_definitions = cJSON_GetObjectItem(json_root, "plugin_definitions");
    if (!cJSON_IsArray(json_plugin_definitions))
    {
        printf("Plugin definitions object is not an array\n");
        return -1;
    }

    int plugin_definitions_len = 0;
    cJSON *json_plugin_definition = NULL;
    cJSON_ArrayForEach(json_plugin_definition, json_plugin_definitions)
    {
        if (plugin_definitions_len >= PLUGIN_REGISTRY_MAX_PLUGIN_LEN)
        {
            printf("Error - reached limit of plugin definitions: %d", PLUGIN_REGISTRY_MAX_PLUGIN_LEN);
            break;
        }

        TODO("Make helper functions for different types (like string)")
        TODO("Add error handling")
        const cJSON *json_plugin_name = cJSON_GetObjectItem(json_plugin_definition, "name");
        if (cJSON_IsString(json_plugin_name))
        {
            snprintf(plugin_registry->plugin_definitions[plugin_definitions_len].plugin_name, PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN,
                     "%s", json_plugin_name->valuestring);
        }

        const cJSON *json_plugin_path = cJSON_GetObjectItem(json_plugin_definition, "path");
        if (cJSON_IsString(json_plugin_path))
        {
            snprintf(plugin_registry->plugin_definitions[plugin_definitions_len].path, PLUGIN_REGISTRY_MAX_PLUGIN_PATH_LEN,
                     "%s", json_plugin_path->valuestring);
        }

        const cJSON *json_plugin_api = cJSON_GetObjectItem(json_plugin_definition, "api");
        if (cJSON_IsString(json_plugin_api))
        {
            snprintf(plugin_registry->plugin_definitions[plugin_definitions_len].api_name, PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN,
                     "%s", json_plugin_api->valuestring);
        }

        plugin_definitions_len++;
    }
    plugin_registry->plugin_definitions_len = plugin_definitions_len;

    cJSON_Delete(json_root);
    return 0;
}