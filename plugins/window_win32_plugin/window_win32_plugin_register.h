#pragma once



#include <window_interface_window_event.h>

#pragma pack(push, 8)

struct LoggerInterface;
struct EnvironmentInterface;

#define WINDOW_WIN32_PLUGIN_WINDOW_EVENTS_SIZE 256

typedef struct WindowInterfaceContext {
    struct LoggerInterface *logger;
    struct EnvironmentInterface *environment;

    struct WindowEvent window_events[WINDOW_WIN32_PLUGIN_WINDOW_EVENTS_SIZE];
    uint32_t window_events_head;
    uint32_t window_events_tail;
} WindowInterfaceContext;

#pragma pack(pop)

