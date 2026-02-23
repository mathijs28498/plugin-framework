#pragma once
#ifndef INPUT_PLUGIN_REGISTER
#define INPUT_PLUGIN_REGISTER

#include <stdbool.h>
#include <stdint.h>
#include <plugin_manager_common.h>
#include <window_interface_window_event_enums.h>

#pragma pack(push, 8)

typedef struct InputInterfaceContext
{
    uint32_t key_state_previous[BITFIELD_SIZE_32(WINDOW_EVENT_KEY_MAX)];
    uint32_t key_state_current[BITFIELD_SIZE_32(WINDOW_EVENT_KEY_MAX)];
} InputInterfaceContext;

#pragma pack(pop)

#endif // #ifndef INPUT_PLUGIN_REGISTER