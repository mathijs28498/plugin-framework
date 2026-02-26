#include <plugin_framework.h>
#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(main, LOG_LEVEL_DEBUG)
#include <environment_interface.h>
#include <window_interface.h>
#include <gui_application_interface.h>

#include <stdio.h>
#include <stdbool.h>

TODO("Make this work")
#define PLUGIN_FRAMEWORK_PLUGINS_LIST(X) \
    X("gui_application", NULL)

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

    if (ret < 0)
    {
        return ret;
    }

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