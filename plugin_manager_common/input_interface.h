#pragma once



#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 8)

struct InputInterfaceContext;
struct WindowEvent;
enum WindowEventKey;

typedef struct InputInterface
{
    struct InputInterfaceContext *context;

    int32_t (*prepare_processing)(struct InputInterfaceContext *context);
    int32_t (*process_window_event)(struct InputInterfaceContext *context, struct WindowEvent *window_event);
    bool (*key_pressed)(struct InputInterfaceContext *context, enum WindowEventKey key);
    bool (*key_held)(struct InputInterfaceContext *context, enum WindowEventKey key);
    bool (*key_released)(struct InputInterfaceContext *context, enum WindowEventKey key);
} InputInterface;

#pragma pack(pop)

#define KEY_PRESSED(interface, key) (interface->key_pressed(interface->context, key))
#define KEY_HELD(interface, key) (interface->key_held(interface->context, key))
#define KEY_RELEASED(interface, key) (interface->key_released(interface->context, key))

