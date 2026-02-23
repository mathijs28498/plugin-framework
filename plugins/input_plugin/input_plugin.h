#pragma once



#include <stdint.h>
#include <stdbool.h>

struct InputInterfaceContext;
struct WindowEvent;
enum WindowEventKey;

int32_t input_plugin_prepare_processing(struct InputInterfaceContext *context);
int32_t input_plugin_process_window_event(struct InputInterfaceContext *context, struct WindowEvent *window_event);

bool input_plugin_key_pressed(struct InputInterfaceContext *context, enum WindowEventKey key);
bool input_plugin_key_held(struct InputInterfaceContext *context, enum WindowEventKey key);
bool input_plugin_key_released(struct InputInterfaceContext *context, enum WindowEventKey key);

