#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <plugin_utils.h>

#include "window_interface_window_event.h"

#define WINDOW_INTERFACE_MAX_WINDOW_NAME_LEN 256

#pragma pack(push, 8)
struct WindowContext;
typedef struct WindowInterfaceCreateWindowOptions
{
    const char window_name[WINDOW_INTERFACE_MAX_WINDOW_NAME_LEN];
} WindowInterfaceCreateWindowOptions;

TODO("See if this should be a separate WindowRendererVtable that extends the WindowVtable")
TODO("Allow for multiple windows")
typedef struct WindowInterfaceOSHandles
{
    void *window_handle;
    void *platform_context_handle;
}WindowInterfaceOSHandles;

typedef struct WindowVtable
{
    WindowInterfaceOSHandles (*get_os_handles)(struct WindowContext *context);

    int32_t (*create_window)(struct WindowContext *context, WindowInterfaceCreateWindowOptions *options);
    int32_t (*get_window_size)(struct WindowContext *context, uint32_t *width, uint32_t *height);
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

static inline WindowInterfaceOSHandles window_get_os_handles(WindowInterface *iface)
{
    return iface->vtable->get_os_handles(iface->context);
}

static inline int32_t window_create_window(WindowInterface *iface, WindowInterfaceCreateWindowOptions *options)
{
    return iface->vtable->create_window(iface->context, options);
}

static inline int32_t window_get_window_size(WindowInterface *iface, uint32_t *width, uint32_t *height)
{
    return iface->vtable->get_window_size(iface->context, width, height);
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
