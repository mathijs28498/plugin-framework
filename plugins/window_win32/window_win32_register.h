#pragma once

#include <window_interface_window_event.h>

#include "plugin_dependencies.h"

#ifndef _WINDEF_ // Prevent redefinition if windows.h is included later
struct HWND__;
typedef struct HWND__ *HWND;
#endif

#pragma pack(push, 8)

#define WINDOW_WIN32_WINDOW_EVENTS_SIZE 256

typedef struct WindowContext
{
    PLUGIN_CONTEXT_DEPENDENCIES

    struct WindowEvent window_events[WINDOW_WIN32_WINDOW_EVENTS_SIZE];
    uint32_t window_events_head;
    uint32_t window_events_tail;
    HWND hwnd;
} WindowContext;

#pragma pack(pop)
