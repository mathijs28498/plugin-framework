#include "window_win32_register.h"

#include <window_interface.h>

#include "window_win32.h"
#include "window_win32_window_events.h"

static const WindowVtable plugin_vtable = {
    .get_os_handles = window_win32_get_os_handles,

    .create_window = window_win32_create_window,
    .get_window_size = window_win32_get_window_size,
    .close_window = window_win32_close_window,

    .poll_os_events = window_win32_poll_os_events,
    .wait_for_os_events = window_win32_wait_for_os_events,
    .pop_window_event = window_win32_window_events_pop,
};

#include "plugin_register.c.inc"