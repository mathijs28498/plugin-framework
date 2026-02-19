#pragma once



#include <stdbool.h>
#include <stdint.h>
#include "window_api_window_event_enums.h"

#pragma pack(push, 8)

typedef enum WindowEventType
{
    WINDOW_EVENT_TYPE_QUIT,
    WINDOW_EVENT_TYPE_KEY_PRESS,
    WINDOW_EVENT_TYPE_MOUSE_PRESS,
    WINDOW_EVENT_TYPE_MOUSE_MOVE,
    WINDOW_EVENT_TYPE_MOUSE_SCROLL,
} WindowEventType;

typedef struct WindowEvent
{
    WindowEventType type;
    union
    {
        struct
        {
            WindowEventKey key;
            bool is_pressed;
        } key_press;
    } data;
} WindowEvent;

#pragma pack(pop)

