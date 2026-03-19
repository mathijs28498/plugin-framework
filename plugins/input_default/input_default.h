#pragma once

#include <stdint.h>
#include <stdbool.h>

struct InputContext;
struct WindowEvent;
enum WindowEventKey;

int32_t input_default_prepare_processing(struct InputContext *context);
int32_t input_default_process_window_event(struct InputContext *context, struct WindowEvent *window_event);

bool input_default_key_pressed(struct InputContext *context, enum WindowEventKey key);
bool input_default_key_held(struct InputContext *context, enum WindowEventKey key);
bool input_default_key_released(struct InputContext *context, enum WindowEventKey key);

