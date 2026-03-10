#pragma once

#include "plugin_utils.h"

TODO("Add optional dependencies")

#if defined(WIN32) && defined(PLUGIN_BUILD_SHARED)
#define PLUGIN_FUNCTION_PREFIX __declspec(dllexport)
#elif defined(WIN32) && defined(PLUGIN_USE_SHARED)
#else
#define PLUGIN_FUNCTION_PREFIX
#endif // defined(WIN32) && defined(PLUGIN_BUILD_SHARED)

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#define PLUGIN_REGISTER_DEPENDENCY(context_type, interface_var_name, interface_name)                          \
    PLUGIN_FUNCTION_PREFIX void CONCAT(PLUGIN_TARGET_NAME, _set_##interface_name)(void *context, void *iface) \
    {                                                                                                         \
        ((context_type *)context)->interface_var_name = iface;                                                \
    }

#define PLUGIN_REGISTER_INTERFACE(get_interface_fn)                               \
    PLUGIN_FUNCTION_PREFIX void *CONCAT(PLUGIN_TARGET_NAME, _get_interface)(void) \
    {                                                                             \
        return get_interface_fn();                                                \
    }

#define PLUGIN_REGISTER_INIT(init_fn)                                               \
    PLUGIN_FUNCTION_PREFIX int32_t CONCAT(PLUGIN_TARGET_NAME, _init)(void *context) \
    {                                                                               \
        return init_fn(context);                                                    \
    }

#define PLUGIN_REGISTER_SHUTDOWN(shutdown_fn)                                           \
    PLUGIN_FUNCTION_PREFIX int32_t CONCAT(PLUGIN_TARGET_NAME, _shutdown)(void *context) \
    {                                                                                   \
        return shutdown_fn(context);                                                    \
    }