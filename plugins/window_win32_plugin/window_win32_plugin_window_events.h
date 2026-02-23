#pragma once

#include <stdbool.h>

struct WindowInterfaceContext;
struct WindowEvent;

bool window_win32_plugin_window_events_pop(struct WindowInterfaceContext *context, struct WindowEvent *window_event);
void window_win32_plugin_window_events_push(struct WindowInterfaceContext *context, struct WindowEvent const *window_event);

