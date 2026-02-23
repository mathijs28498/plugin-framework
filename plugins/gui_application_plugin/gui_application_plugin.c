#include "gui_application_plugin.h"

#include <stdint.h>

#include <window_interface.h>
#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(gui_application_plugin, LOG_LEVEL_DEBUG)
#include <input_interface.h>
#include <plugin_manager_common.h>
#include <draw_interface.h>

#include "gui_application_plugin_register.h"

int32_t gui_application_plugin_run(GuiApplicationInterfaceContext *context)
{
    WindowInterface *window= context->window;
    LoggerInterface *logger= context->logger;
    InputInterface *input= context->input;
    DrawInterface *draw= context->draw;

    LOG_INF(logger, "Starting main loop");

    bool gui_application_running = true;
    while (gui_application_running)
    {
        window->poll_os_events(window->context);
        WindowEvent window_event;

        input->prepare_processing(input->context);
        while (window->pop_window_event(window->context, &window_event))
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
                input->process_window_event(input->context, &window_event);
                break;
            }
        }
        if (!gui_application_running)
        {
            break;
        }

        if (KEY_PRESSED(input, WINDOW_EVENT_KEY_ESCAPE))
        {
            window->close_window(window->context);
        }

        // logic->update(logic->context);
        // if (gui_applicaiton_do_fixed_update(context))
        // {
        //     logic->fixed_update(logic->context);
        // }

          
        draw->present(draw->context);

    }

    return 0;
}
