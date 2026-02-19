#pragma once



#include <stdint.h>

struct WindowApiContext;
struct WindowApiCreateWindowOptions;
struct WindowEvent;

int32_t window_win32_plugin_create_window(struct WindowApiContext *context, struct WindowApiCreateWindowOptions *options);
int32_t window_win32_plugin_close_window(struct WindowApiContext *context);

int32_t window_win32_plugin_poll_os_events(struct WindowApiContext *context);
int32_t window_win32_plugin_wait_for_os_events(struct WindowApiContext *context);

