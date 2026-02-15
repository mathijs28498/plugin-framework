#pragma once
#ifndef PLUGIN_MANAGER_IMPL_H
#define PLUGIN_IMPL_H

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#define PLUGIN_DEPENDENCY_STRING(api_type, api_var_name, api_name) #api_name,
#define PLUGIN_DEPENDENCY_SETTER(api_type, api_var_name, api_name)                                         \
    __declspec(dllexport) void CONCAT(PLUGIN_API_NAME, _set_##api_name)(void *void_context, api_type *api) \
    {                                                                                                      \
        __PLUGIN_CONTEXT_TYPE *context = (__PLUGIN_CONTEXT_TYPE *)void_context;                            \
        context->api_var_name = api;                                                                       \
    }

#define PLUGIN_REGISTER_DEPENDENCIES(CONTEXT_TYPE, DEPENDENCY_LIST)                               \
    typedef CONTEXT_TYPE __PLUGIN_CONTEXT_TYPE;                                                   \
    __declspec(dllexport) void                                                                    \
    CONCAT(PLUGIN_API_NAME, _get_dependencies)(const char *const **dependencies, uint32_t *count) \
    {                                                                                             \
        static const char *const plugin_dependencies[] = {                                        \
            DEPENDENCY_LIST(PLUGIN_DEPENDENCY_STRING)};                                           \
                                                                                                  \
        *dependencies = plugin_dependencies;                                                      \
        *count = (uint32_t)(sizeof(plugin_dependencies) / sizeof(plugin_dependencies[0]));        \
    }                                                                                             \
    DEPENDENCY_LIST(PLUGIN_DEPENDENCY_SETTER)

#define PLUGIN_REGISTER_API(get_api_fn, api_type)                           \
    __declspec(dllexport) api_type *CONCAT(PLUGIN_API_NAME, _get_api)(void) \
    {                                                                       \
        return get_api_fn();                                                \
    }

#define PLUGIN_REGISTER_INIT(init_fn, context_type)                                      \
    __declspec(dllexport) int32_t CONCAT(PLUGIN_API_NAME, _init)(context_type * context) \
    {                                                                                    \
        return init_fn(context);                                                         \
    }

#endif // #ifndef PLUGIN_IMPL_H