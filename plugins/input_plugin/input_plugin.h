#pragma once



#include <stdint.h>
#include <stdbool.h>

struct InputApiContext;
struct WindowEvent;
enum WindowEventKey;

int32_t input_plugin_prepare_processing(struct InputApiContext *context);
int32_t input_plugin_process_window_event(struct InputApiContext *context, struct WindowEvent *window_event);

bool input_plugin_key_pressed(struct InputApiContext *context, enum WindowEventKey key);
bool input_plugin_key_held(struct InputApiContext *context, enum WindowEventKey key);
bool input_plugin_key_released(struct InputApiContext *context, enum WindowEventKey key);

