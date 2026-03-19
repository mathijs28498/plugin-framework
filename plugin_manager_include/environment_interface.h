#pragma once

#include "plugin_utils.h"

#pragma pack(push, 8)

struct EnvironmentContext;


typedef struct EnvironmentVtable
{
    void (*get_args)(struct EnvironmentContext *context, int *argc, char ***argv);
    void (*get_platform_context)(struct EnvironmentContext *context, void **platform_context);
} EnvironmentVtable;

typedef struct EnvironmentInterface
{
    struct EnvironmentContext *context;
    EnvironmentVtable *vtable;
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

static inline void environment_get_args(EnvironmentInterface *iface, int *argc, char ***argv)
{
    iface->vtable->get_args(iface->context, argc, argv);
}

static inline void environment_get_platform_context(struct EnvironmentInterface *iface, void **platform_context)
{
    iface->vtable->get_platform_context(iface->context, platform_context);
}