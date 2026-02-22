#include "gui_application_plugin.h"

#include <stdint.h>

#include <window_api.h>
#include <logger_api.h>
LOGGER_API_REGISTER(gui_application_plugin, LOG_LEVEL_DEBUG)
#include <input_api.h>
#include <plugin_manager_common.h>
#include <draw_api.h>
#include <logic_api.h>

#include "gui_application_plugin_register.h"

int32_t gui_application_plugin_run(GuiApplicationApiContext *context)
{
    WindowApi *window_api = context->window_api;
    LoggerApi *logger_api = context->logger_api;
    InputApi *input_api = context->input_api;
    DrawApi *draw_api = context->draw_api;

    LOG_INF(logger_api, "Starting main loop");

    bool gui_application_running = true;
    while (gui_application_running)
    {
        window_api->poll_os_events(window_api->context);
        WindowEvent window_event;

        input_api->prepare_processing(input_api->context);
        while (window_api->pop_window_event(window_api->context, &window_event))
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
                input_api->process_window_event(input_api->context, &window_event);
                break;
            }
        }
        if (!gui_application_running)
        {
            break;
        }

        if (KEY_PRESSED(input_api, WINDOW_EVENT_KEY_ESCAPE))
        {
            window_api->close_window(window_api->context);
        }

        // logic_api->update(logic_api->context);
        // if (gui_applicaiton_do_fixed_update(context))
        // {
        //     logic_api->fixed_update(logic_api->context);
        // }

          
        draw_api->present(draw_api->context);

    }

    return 0;
}
