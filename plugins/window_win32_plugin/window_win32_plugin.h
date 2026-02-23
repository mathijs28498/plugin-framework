#pragma once



#include <stdint.h>

struct WindowInterfaceContext;
struct WindowInterfaceCreateWindowOptions;
struct WindowEvent;

int32_t window_win32_plugin_create_window(struct WindowInterfaceContext *context, struct WindowInterfaceCreateWindowOptions *options);
int32_t window_win32_plugin_close_window(struct WindowInterfaceContext *context);

int32_t window_win32_plugin_poll_os_events(struct WindowInterfaceContext *context);
int32_t window_win32_plugin_wait_for_os_events(struct WindowInterfaceContext *context);

