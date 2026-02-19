#pragma once



#include <window_api_window_event.h>

#pragma pack(push, 8)

struct LoggerApi;
struct EnvironmentApi;

#define WINDOW_WIN32_PLUGIN_WINDOW_EVENTS_SIZE 256

typedef struct WindowApiContext {
    struct LoggerApi *logger_api;
    struct EnvironmentApi *environment_api;

    struct WindowEvent window_events[WINDOW_WIN32_PLUGIN_WINDOW_EVENTS_SIZE];
    uint32_t window_events_head;
    uint32_t window_events_tail;
} WindowApiContext;

#pragma pack(pop)

