#pragma once



#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 8)

struct InputApiContext;
struct WindowEvent;
enum WindowEventKey;

typedef struct InputApi
{
    struct InputApiContext *context;

    int32_t (*prepare_processing)(struct InputApiContext *context);
    int32_t (*process_window_event)(struct InputApiContext *context, struct WindowEvent *window_event);
    bool (*key_pressed)(struct InputApiContext *context, enum WindowEventKey key);
    bool (*key_held)(struct InputApiContext *context, enum WindowEventKey key);
    bool (*key_released)(struct InputApiContext *context, enum WindowEventKey key);
} InputApi;

#pragma pack(pop)

#define KEY_PRESSED(api, key) (api->key_pressed(api->context, key))
#define KEY_HELD(api, key) (api->key_held(api->context, key))
#define KEY_RELEASED(api, key) (api->key_released(api->context, key))

