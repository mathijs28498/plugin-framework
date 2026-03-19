#include "input_default.h"

#include <window_interface_window_event.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "input_default_register.h"

void get_key_state_bitfield_indices(WindowEventKey key, size_t *key_state_index, size_t *key_state_bit_index)
{
    *key_state_index = key / 32;
    *key_state_bit_index = key % 32;
}

void get_key_state(InputContext *context, WindowEventKey key, bool *previous, bool *current)
{
    size_t key_state_index;
    size_t key_state_bit_index;
    get_key_state_bitfield_indices(key, &key_state_index, &key_state_bit_index);
    *previous = (context->key_state_previous[key_state_index] & (1 << key_state_bit_index)) != 0;
    *current = (context->key_state_current[key_state_index] & (1 << key_state_bit_index)) != 0;
}

int32_t input_default_prepare_processing(InputContext *context)
{
    memcpy(context->key_state_previous, context->key_state_current, ARRAY_SIZE(context->key_state_previous));
    return 0;
}

int32_t input_default_process_window_event(InputContext *context, WindowEvent *window_event)
{
    switch (window_event->type)
    {
    case WINDOW_EVENT_TYPE_KEY_PRESS:
        WindowEventKey key = window_event->data.key_press.key;
        size_t key_state_index, key_state_bit_index;
        get_key_state_bitfield_indices(key, &key_state_index, &key_state_bit_index);

        uint32_t bit_mask = 1U << key_state_bit_index;
        if (window_event->data.key_press.is_pressed)
        {
            context->key_state_current[key_state_index] |= bit_mask;
        }
        else
        {
            context->key_state_current[key_state_index] &= ~bit_mask;
        }
        break;
    default:
        break;
    }
    return 0;
}

bool input_default_key_pressed(InputContext *context, WindowEventKey key)
{
    bool previous, current;
    get_key_state(context, key, &previous, &current);
    return !previous && current;
}

bool input_default_key_held(InputContext *context, WindowEventKey key)
{
    bool previous, current;
    get_key_state(context, key, &previous, &current);
    return current;
}

bool input_default_key_released(InputContext *context, WindowEventKey key)
{
    bool previous, current;
    get_key_state(context, key, &previous, &current);
    return previous && !current;
}