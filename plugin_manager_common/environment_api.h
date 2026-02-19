#pragma once

#include "plugin_manager_common.h"

#pragma pack(push, 8)

struct EnvironmentApiContext;

typedef struct EnvironmentApi
{
    struct EnvironmentApiContext *context;

    void (*get_args)(struct EnvironmentApiContext *context, int *argc, char ***argv);
    void (*get_platform_context)(struct EnvironmentApiContext *context, void **platform_context);
} EnvironmentApi;

#if WINDOWS_GUI
struct HINSTANCE__;
typedef struct WindowsPlatformContext
{
    struct HINSTANCE__ *hInstance;
    int nCmdShow;
} WindowsPlatformContext;

#define ENVIRONMENT_API_GET_WINDOWS_CONTEXT(environment_api, windows_context) environment_api->get_platform_context(environment_api->context, (void **)windows_context)
#endif // #if WINDOWS_GUI

#pragma pack(pop)
