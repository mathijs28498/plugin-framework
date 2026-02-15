#pragma once
#ifndef WINDOW_WIN32_PLUGIN_H
#define WINDOW_WIN32_PLUGIN_H

#include <stdint.h>

struct WindowApiContext;
struct WindowApiCreateWindowOptions;

int32_t window_win32_plugin_create_window(struct WindowApiContext *context, struct WindowApiCreateWindowOptions *options);
int32_t window_win32_plugin_poll_events(struct WindowApiContext *context);
int32_t window_win32_plugin_wait_for_events(struct WindowApiContext *context);

#endif // #ifndef WINDOW_WIN32_PLUGIN_H