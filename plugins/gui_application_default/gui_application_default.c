#include "gui_application_default.h"

#include <stdint.h>

#include <window_interface.h>
#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(gui_application_default, LOG_LEVEL_DEBUG)
#include <input_interface.h>
#include <plugin_utils.h>
#include <draw_interface.h>
#include <renderer_interface.h>

#include "gui_application_default_register.h"

#define GUI_APPLICATION_DEFAULT_MAX_WINDOW_EVENTS_PER_FRAME 256

int32_t gui_application_default_setup(GuiApplicationContext *context, WindowInterfaceCreateWindowOptions *create_window_options)
{
    int32_t ret;
    ret = window_create_window(context->window, create_window_options);
    // ret = renderer_init(context->renderer);

    return 0;
}

int32_t gui_application_default_run(GuiApplicationContext *context)
{
    LoggerInterface *logger = context->logger;

    LOG_INF("Starting main loop");

    bool gui_application_running = true;
    while (gui_application_running)
    {
        window_poll_os_events(context->window);
        WindowEvent window_event;

        input_prepare_processing(context->input);
        SAFE_WHILE(
            window_pop_window_event(context->window, &window_event),
            GUI_APPLICATION_DEFAULT_MAX_WINDOW_EVENTS_PER_FRAME,
            {
                LOG_WRN("Too many window events in one frame (%d), skipping events till next frame", GUI_APPLICATION_DEFAULT_MAX_WINDOW_EVENTS_PER_FRAME);
            })
        {
            switch (window_event.type)
            {
            case WINDOW_EVENT_TYPE_QUIT:
                gui_application_running = false;
                break;
            case WINDOW_EVENT_TYPE_KEY_PRESS:
            case WINDOW_EVENT_TYPE_MOUSE_MOVE:
            case WINDOW_EVENT_TYPE_MOUSE_PRESS:
            case WINDOW_EVENT_TYPE_MOUSE_SCROLL:
                input_process_window_event(context->input, &window_event);
                break;
            }
        }
        if (!gui_application_running)
        {
            break;
        }

        if (input_key_pressed(context->input, WINDOW_EVENT_KEY_ESCAPE))
        {
            LOG_DBG("Closing application");
            window_close_window(context->window);
        }

        // logic->update(logic->context);
        // if (gui_applicaiton_do_fixed_update(context))
        // {
        //     logic->fixed_update(logic->context);
        // }

        draw_present(context->draw);
    }

    return 0;
}
