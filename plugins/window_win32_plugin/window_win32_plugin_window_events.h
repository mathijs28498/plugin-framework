#pragma once



#include <stdbool.h>

struct WindowApiContext;
struct WindowEvent;

bool window_win32_plugin_window_events_pop(struct WindowApiContext *context, struct WindowEvent *window_event);
void window_win32_plugin_window_events_push(struct WindowApiContext *context, struct WindowEvent const *window_event);

