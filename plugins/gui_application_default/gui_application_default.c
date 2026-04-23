#include "gui_application_default.h"

#include <stdint.h>

#include <plugin_sdk/window/v1/window_interface.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(gui_application_default, LOG_LEVEL_DEBUG)
#include <plugin_sdk/input/v1/input_interface.h>
#include <plugin_sdk/plugin_utils.h>
#include <plugin_sdk/draw/v1/draw_interface.h>
#include <plugin_sdk/renderer/v1/renderer_interface.h>

#include "gui_application_default_register.h"

#define GUI_APPLICATION_DEFAULT_MAX_WINDOW_EVENTS_PER_FRAME 256

int32_t gui_application_default_setup(GuiApplicationContext *context, WindowInterfaceCreateWindowOptions *create_window_options)
{
    int32_t ret;
    RETURN_IF_ERROR(context->deps.logger, ret, window_create_window(context->deps.window, create_window_options),
                    "Failed to create window: %d", ret);
    RETURN_IF_ERROR(context->deps.logger, ret, draw_start(context->deps.draw),
                    "Failed to start draw: %d", ret);

    return 0;
}

int32_t gui_application_default_run(GuiApplicationContext *context)
{
    int32_t ret;

    LOG_INF_TRACE(context->deps.logger, "Starting main loop");

    bool gui_application_running = true;
    while (gui_application_running)
    {
        RETURN_IF_ERROR(context->deps.logger, ret, window_poll_os_events(context->deps.window),
                        "Failed to poll os events: %d", ret);
        WindowEvent window_event;

        RETURN_IF_ERROR(context->deps.logger, ret, input_prepare_processing(context->deps.input),
                        "Failed to prepare input processing: %d", ret);

        SAFE_WHILE(
            window_pop_window_event(context->deps.window, &window_event),
            GUI_APPLICATION_DEFAULT_MAX_WINDOW_EVENTS_PER_FRAME,
            {
                LOG_WRN_TRACE(context->deps.logger, "Too many window events in one frame (%d), skipping events till next frame", GUI_APPLICATION_DEFAULT_MAX_WINDOW_EVENTS_PER_FRAME);
            })
        {
            switch (window_event.type)
            {
            case WINDOW_EVENT_TYPE_QUIT:
                gui_application_running = false;
                break;
            case WINDOW_EVENT_TYPE_RESIZE:
                draw_on_window_resize(context->deps.draw, window_event.resize.width, window_event.resize.height);
                break;
            case WINDOW_EVENT_TYPE_KEY_PRESS:
            case WINDOW_EVENT_TYPE_MOUSE_MOVE:
            case WINDOW_EVENT_TYPE_MOUSE_PRESS:
            case WINDOW_EVENT_TYPE_MOUSE_SCROLL:
                RETURN_IF_ERROR(context->deps.logger, ret, input_process_window_event(context->deps.input, &window_event),
                                "Failed to process window event: %d", ret);
                break;
            default:
                LOG_WRN_TRACE(context->deps.logger, "Unrecognized event: %d", window_event.type);
                break;
            }
        }
        if (!gui_application_running)
        {
            break;
        }

        if (input_key_pressed(context->deps.input, WINDOW_EVENT_KEY_ESCAPE))
        {
            LOG_DBG_TRACE(context->deps.logger, "Closing application");
            RETURN_IF_ERROR(context->deps.logger, ret, window_close_window(context->deps.window),
                            "Failed to close window: %d", ret);
            break;
        }

        // logic->update(logic->context);
        // if (gui_application_do_fixed_update(context))
        // {
        //     logic->fixed_update(logic->context);
        // }

        RETURN_IF_ERROR(context->deps.logger, ret, draw_present(context->deps.draw),
                        "Failed to present draw: %d", ret);
        TODO("If returns 1 wait for events ");
    }

    return 0;
}
