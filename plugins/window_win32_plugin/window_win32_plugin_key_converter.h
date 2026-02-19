#pragma once



#include <stdint.h>

enum WindowEventKey;

enum WindowEventKey win32_key_to_window_event_key(uintptr_t wparam);

