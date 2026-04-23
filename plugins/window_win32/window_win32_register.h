#pragma once

#include <plugin_sdk/window/v1/window_interface_window_event.h>
#include <plugin_sdk/plugin_utils.h>

#include "plugin_dependencies.h"

#ifndef _WINDEF_ // Prevent redefinition if windows.h is included later
struct HWND__;
typedef struct HWND__ *HWND;
#endif

#pragma pack(push, 8)

TODO("Fix this differently")
#define WINDOW_WIN32_WINDOW_EVENTS_SIZE 32

typedef struct WindowContext
{
    PluginDependencies deps;

    struct WindowEvent window_events[WINDOW_WIN32_WINDOW_EVENTS_SIZE];
    uint32_t window_events_head;
    uint32_t window_events_tail;
    HWND hwnd;
} WindowContext;

#pragma pack(pop)
