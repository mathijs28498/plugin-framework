#pragma once
#ifndef PLUGIN_API_H
#define PLUGIN_API_H

// TODO: Add functionality for static loading (make the add/add_search_path into macros)

#include <stdint.h>

typedef struct PluginApi
{
    int32_t (*init)();
    int32_t (*add)(const char* plugin_name, const void **out_plugin_interface);
    int32_t (*get)(const char* plugin_name, const void **out_plugin_interface);
} PluginApi;

#endif // #ifndef PLUGIN_API_H