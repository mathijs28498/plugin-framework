#include "window_win32_plugin_window_events.h"

#include <stdbool.h>
#include <string.h>

#include <plugin_manager_common.h>

#include "window_win32_plugin_register.h"

bool window_win32_plugin_window_events_pop(WindowInterfaceContext *context, WindowEvent *window_event)
{
    if (context->window_events_head == context->window_events_tail)
    {
        return false;
    }

    memcpy(window_event, &context->window_events[context->window_events_head], sizeof(*window_event));
    context->window_events_head = (context->window_events_head + 1) % ARRAY_SIZE(context->window_events);

    return true;
}

void window_win32_plugin_window_events_push(WindowInterfaceContext *context, WindowEvent const *window_event)
{
    memcpy(&context->window_events[context->window_events_tail], window_event, sizeof(*window_event));
    context->window_events_tail = (context->window_events_tail + 1) % ARRAY_SIZE(context->window_events);
}