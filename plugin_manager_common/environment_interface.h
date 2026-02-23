#pragma once

#include "plugin_manager_common.h"

#pragma pack(push, 8)

struct EnvironmentInterfaceContext;

typedef struct EnvironmentInterface
{
    struct EnvironmentInterfaceContext *context;

    void (*get_args)(struct EnvironmentInterfaceContext *context, int *argc, char ***argv);
    void (*get_platform_context)(struct EnvironmentInterfaceContext *context, void **platform_context);
} EnvironmentInterface;

#if WINDOWS_GUI
struct HINSTANCE__;
typedef struct WindowsPlatformContext
{
    struct HINSTANCE__ *hInstance;
    int nCmdShow;
} WindowsPlatformContext;

#define ENVIRONMENT_INTERFACE_GET_WINDOWS_CONTEXT(environment_interface, windows_context) environment_interface->get_platform_context(environment_interface->context, (void **)windows_context)
#endif // #if WINDOWS_GUI

#pragma pack(pop)
