#pragma once

#include "plugin_manager_common.h"

TODO("Add optional dependencies")

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#define PLUGIN_DEPENDENCY_STRING(interface_type, interface_var_name, interface_name) #interface_name,
#define PLUGIN_DEPENDENCY_SETTER(interface_type, interface_var_name, interface_name)                                           \
    __declspec(dllexport) void CONCAT(PLUGIN_INTERFACE_NAME, _set_##interface_name)(void *void_context, interface_type *iface) \
    {                                                                                                                          \
        __PLUGIN_CONTEXT_TYPE *context = (__PLUGIN_CONTEXT_TYPE *)void_context;                                                \
        context->interface_var_name = iface;                                                                                   \
    }

#define PLUGIN_REGISTER_DEPENDENCIES(CONTEXT_TYPE, DEPENDENCY_LIST)                                     \
    typedef CONTEXT_TYPE __PLUGIN_CONTEXT_TYPE;                                                         \
    __declspec(dllexport) void                                                                          \
    CONCAT(PLUGIN_INTERFACE_NAME, _get_dependencies)(const char *const **dependencies, uint32_t *count) \
    {                                                                                                   \
        static const char *const plugin_dependencies[] = {                                              \
            DEPENDENCY_LIST(PLUGIN_DEPENDENCY_STRING)};                                                 \
                                                                                                        \
        *dependencies = plugin_dependencies;                                                            \
        *count = (uint32_t)(sizeof(plugin_dependencies) / sizeof(plugin_dependencies[0]));              \
    }                                                                                                   \
    DEPENDENCY_LIST(PLUGIN_DEPENDENCY_SETTER)

#define PLUGIN_REGISTER_INTERFACE(get_interface_fn, interface_type)                           \
    __declspec(dllexport) interface_type *CONCAT(PLUGIN_INTERFACE_NAME, _get_interface)(void) \
    {                                                                                         \
        return get_interface_fn();                                                            \
    }

#define PLUGIN_REGISTER_INIT(init_fn, context_type)                                            \
    __declspec(dllexport) int32_t CONCAT(PLUGIN_INTERFACE_NAME, _init)(context_type * context) \
    {                                                                                          \
        return init_fn(context);                                                               \
    }
