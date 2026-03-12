#include <stdio.h>
#include <stdbool.h>

#include <plugin_framework.h>
#include <input_interface.h>
#include <draw_2d_interface.h>
#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(main, LOG_LEVEL_DEBUG)
#include <input_interface.h>
#include <environment_interface.h>
#include <window_interface.h>
#include <gui_application_interface.h>

PLUGIN_FRAMEWORK_MAIN()
{
    int32_t ret = 0;

    // ret = PLUGIN_FRAMEWORK_ADD("logic_ecs", NULL);
    // ret = PLUGIN_MANAGER_LINK_INTERFACES("logic", "logic_interfaces",
    //                                "logic_ecs");

    // ret = PLUGIN_FRAMEWORK_ADD("draw", NULL);
    // ret = PLUGIN_FRAMEWORK_ADD("draw_2d", NULL);
    // ret = PLUGIN_FRAMEWORK_ADD("draw_3d", NULL);
    // ret = PLUGIN_FRAMEWORK_ADD("draw_raytracer", NULL);

    // ret = PLUGIN_MANAGER_LINK_INTERFACES("draw", "draw_interfaces",
    //                                "draw_2d", "draw_3d", "draw_ui");

    LoggerInterface *logger;
    ret = PLUGIN_FRAMEWORK_GET("logger", &logger);

    if (ret < 0)
    {
        return ret;
    }

    LOG_WRN(logger, "This works yes");

    // WindowInterface *window;
    // ret = PLUGIN_FRAMEWORK_GET("window", &window);

    // if (ret < 0)
    // {
    //     return ret;
    // }

    // WindowInterfaceCreateWindowOptions create_window_options = {
    //     .window_name = "My app",
    // };
    // window->create_window(window->context, &create_window_options);
    // while (1)
    // {
    //     window->poll_os_events(window->context);
    //     WindowEvent window_event;
    //     while (window->pop_window_event(window->context, &window_event))
    //     {

    //     }
    // }

    Draw2dInterface *draw_2d;
    ret = PLUGIN_FRAMEWORK_GET("draw_2d", &draw_2d);
    if (ret < 0)
    {
        return ret;
    }

    draw_2d->test(draw_2d->context, 32);


    GuiApplicationInterface *gui_application;
    ret = PLUGIN_FRAMEWORK_GET("gui_application", &gui_application);
    if (ret < 0)
    {
        return ret;
    }

    WindowInterfaceCreateWindowOptions create_window_options = {
        .window_name = "My app",
    };
    ret = gui_application->setup(gui_application->context, &create_window_options);

    ret = gui_application->run(gui_application->context);

    return 0;
}