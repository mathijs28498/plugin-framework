#pragma once

#include <stdbool.h>

struct WindowContext;
struct WindowEvent;

bool window_win32_window_events_pop(struct WindowContext *context, struct WindowEvent *window_event);
void window_win32_window_events_push(struct WindowContext *context, struct WindowEvent const *window_event);

