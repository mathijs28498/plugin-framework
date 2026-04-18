#pragma once

#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 8)

struct InputContext;
struct WindowEvent;
enum WindowEventKey;

typedef struct InputVtable
{
    int32_t (*prepare_processing)(struct InputContext *context);
    int32_t (*process_window_event)(struct InputContext *context, struct WindowEvent *window_event);
    bool (*key_pressed)(struct InputContext *context, enum WindowEventKey key);
    bool (*key_held)(struct InputContext *context, enum WindowEventKey key);
    bool (*key_released)(struct InputContext *context, enum WindowEventKey key);
} InputVtable;

typedef struct InputInterface
{
    struct InputContext *context;
    InputVtable *vtable;
} InputInterface;

#pragma pack(pop)

static inline int32_t input_prepare_processing(InputInterface *iface)
{
    return iface->vtable->prepare_processing(iface->context);
}

static inline int32_t input_process_window_event(InputInterface *iface, struct WindowEvent *window_event)
{
    return iface->vtable->process_window_event(iface->context, window_event);
}

static inline bool input_key_pressed(InputInterface *iface, enum WindowEventKey key)
{
    return iface->vtable->key_pressed(iface->context, key);
}

static inline bool input_key_held(InputInterface *iface, enum WindowEventKey key)
{
    return iface->vtable->key_held(iface->context, key);
}

static inline bool input_key_released(InputInterface *iface, enum WindowEventKey key)
{
    return iface->vtable->key_released(iface->context, key);
}


#define KEY_PRESSED(interface, key) (interface->key_pressed(interface->context, key))
#define KEY_HELD(interface, key) (interface->key_held(interface->context, key))
#define KEY_RELEASED(interface, key) (interface->key_released(interface->context, key))
