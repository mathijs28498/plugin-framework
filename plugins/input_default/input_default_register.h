#pragma once

#include <stdint.h>
#include <plugin_sdk/plugin_utils.h>
#include <plugin_sdk/window/v1/window_interface_window_event_enums.h>

#include "plugin_dependencies.h"

#pragma pack(push, 8)

typedef struct InputContext
{
    uint32_t key_state_previous[BITFIELD_SIZE_32(WINDOW_EVENT_KEY_MAX)];
    uint32_t key_state_current[BITFIELD_SIZE_32(WINDOW_EVENT_KEY_MAX)];
} InputContext;

#pragma pack(pop)