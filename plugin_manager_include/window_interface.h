#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "window_interface_window_event.h"

#define WINDOW_INTERFACE_MAX_WINDOW_NAME_LEN 256

#pragma pack(push, 8)
struct WindowContext;
typedef struct WindowInterfaceCreateWindowOptions
{
    const char window_name[WINDOW_INTERFACE_MAX_WINDOW_NAME_LEN];
} WindowInterfaceCreateWindowOptions;

typedef struct WindowVtable
{
    int32_t (*create_window)(struct WindowContext *context, WindowInterfaceCreateWindowOptions *options);
    int32_t (*close_window)(struct WindowContext *context);
    int32_t (*poll_os_events)(struct WindowContext *context);
    int32_t (*wait_for_os_events)(struct WindowContext *context);
    bool (*pop_window_event)(struct WindowContext *context, WindowEvent *window_event);
} WindowVtable;

typedef struct WindowInterface
{
    struct WindowContext *context;
    WindowVtable *vtable;
} WindowInterface;

#pragma pack(pop)

static inline int32_t window_create_window(WindowInterface *iface, WindowInterfaceCreateWindowOptions *options)
{
    return iface->vtable->create_window(iface->context, options);
}

static inline int32_t window_close_window(WindowInterface *iface)
{
    return iface->vtable->close_window(iface->context);
}

static inline int32_t window_poll_os_events(WindowInterface *iface)
{
    return iface->vtable->poll_os_events(iface->context);
}

static inline int32_t window_wait_for_os_events(WindowInterface *iface)
{
    return iface->vtable->wait_for_os_events(iface->context);
}

static inline bool window_pop_window_event(WindowInterface *iface, WindowEvent *window_event)
{
    return iface->vtable->pop_window_event(iface->context, window_event);
}
