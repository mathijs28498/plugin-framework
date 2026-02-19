#pragma once



#include <stdint.h>
#include <stdbool.h>

#include "window_api_window_event.h"

#define WINDOW_API_MAX_WINDOW_NAME_LEN 256

#pragma pack(push, 8)
struct WindowApiContext;
typedef struct WindowApiCreateWindowOptions
{
    const char window_name[WINDOW_API_MAX_WINDOW_NAME_LEN];
} WindowApiCreateWindowOptions;


typedef struct WindowApi
{
    struct WindowApiContext *context;

    int32_t (*create_window)(struct WindowApiContext *context, WindowApiCreateWindowOptions *options);
    int32_t (*close_window)(struct WindowApiContext *context);
    int32_t (*poll_os_events)(struct WindowApiContext *context);
    int32_t (*wait_for_os_events)(struct WindowApiContext *context);
    bool (*pop_window_event)(struct WindowApiContext *context, WindowEvent *window_event);
} WindowApi;

#pragma pack(pop)

