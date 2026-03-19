#pragma once

#include <stdint.h>

#pragma pack(push, 8)

struct GuiApplicationContext;
struct WindowInterfaceCreateWindowOptions;
typedef struct GuiApplicationVtable
{
    int32_t (*setup)(struct GuiApplicationContext *context, struct WindowInterfaceCreateWindowOptions *create_window_options);
    int32_t (*run)(struct GuiApplicationContext *context);
} GuiApplicationVtable;

typedef struct GuiApplicationInterface {
    struct GuiApplicationContext *context;
    GuiApplicationVtable *vtable;
} GuiApplicationInterface;

#pragma pack(pop)

static inline int32_t gui_application_setup(GuiApplicationInterface *iface, struct WindowInterfaceCreateWindowOptions *create_window_options)
{
    return iface->vtable->setup(iface->context, create_window_options);
}

static inline int32_t gui_application_run(GuiApplicationInterface *iface)
{
    return iface->vtable->run(iface->context);
}

