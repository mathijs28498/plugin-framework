#include <plugin_framework.h>
#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(main, LOG_LEVEL_DEBUG)
#include <environment_interface.h>
#include <window_interface.h>
#include <gui_application_interface.h>

#include <stdio.h>
#include <stdbool.h>

PLUGIN_FRAMEWORK_MAIN()
{
    int32_t ret;

    ret = PLUGIN_FRAMEWORK_ADD("gui_application", NULL);

    // ret = PLUGIN_FRAMEWORK_ADD("logic_ecs", NULL);
    // ret = PLUGIN_MANAGER_LINK_INTERFACES("logic", "logic_interfaces",
    //                                "logic_ecs");

    // ret = PLUGIN_FRAMEWORK_ADD("draw", NULL);
    // ret = PLUGIN_FRAMEWORK_ADD("draw_2d", NULL);
    // ret = PLUGIN_FRAMEWORK_ADD("draw_3d", NULL);
    // ret = PLUGIN_FRAMEWORK_ADD("draw_raytracer", NULL);
    // ret = PLUGIN_FRAMEWORK_ADD("draw_ui", NULL);

    // ret = PLUGIN_MANAGER_LINK_INTERFACES("draw", "draw_interfaces",
    //                                "draw_2d", "draw_3d", "draw_ui");

    ret = PLUGIN_FRAMEWORK_ADD("window", NULL);
    ret = PLUGIN_FRAMEWORK_LOAD();
    if (ret < 0)
    {
        return ret;
    }

    WindowInterface *window;
    GuiApplicationInterface *gui_application;
    // Draw2dInterface *draw_2d;

    ret = PLUGIN_FRAMEWORK_GET("window", &window);
    if (ret < 0)
    {
        return ret;
    }

    PLUGIN_FRAMEWORK_GET("gui_application", &gui_application);
    if (ret < 0)
    {
        return ret;
    }

    // PLUGIN_FRAMEWORK_GET("draw_2d", &draw_2d);
    // if (ret < 0)
    // {
    //     return ret;
    // }

    WindowInterfaceCreateWindowOptions create_window_options = {
        .window_name = "My app",
    };
    ret = window->create_window(window->context, &create_window_options);

    ret = gui_application->run(gui_application->context);

    return 0;
}