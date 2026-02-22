#include <plugin_manager_api.h>
#include <logger_api.h>
LOGGER_API_REGISTER(main, LOG_LEVEL_DEBUG)
#include <environment_api.h>
#include <window_api.h>
#include <gui_application_api.h>

#include <stdio.h>
#include <stdbool.h>

PLUGIN_MANAGER_API_MAIN()
{
    int32_t ret;

    ret = PLUGIN_MANAGER_API_ADD("gui_application_api", NULL);

    // ret = PLUGIN_MANAGER_API_ADD("logic_ecs_api", NULL);
    // ret = PLUGIN_MANAGER_LINK_APIS("logic_api", "logic_apis",
    //                                "logic_ecs_api");

    // ret = PLUGIN_MANAGER_API_ADD("draw_api", NULL);
    // ret = PLUGIN_MANAGER_API_ADD("draw_2d_api", NULL);
    // ret = PLUGIN_MANAGER_API_ADD("draw_3d_api", NULL);
    // ret = PLUGIN_MANAGER_API_ADD("draw_raytracer_api", NULL);
    // ret = PLUGIN_MANAGER_API_ADD("draw_ui_api", NULL);

    // ret = PLUGIN_MANAGER_LINK_APIS("draw_api", "draw_apis",
    //                                "draw_2d_api", "draw_3d_api", "draw_ui_api");

    ret = PLUGIN_MANAGER_API_ADD("window_api", NULL);
    ret = PLUGIN_MANAGER_API_LOAD();
    if (ret < 0)
    {
        return ret;
    }

    WindowApi *window_api;
    GuiApplicationApi *gui_application_api;
    // Draw2dApi *draw_2d_api;

    ret = PLUGIN_MANAGER_API_GET("window_api", &window_api);
    if (ret < 0)
    {
        return ret;
    }

    PLUGIN_MANAGER_API_GET("gui_application_api", &gui_application_api);
    if (ret < 0)
    {
        return ret;
    }

    // PLUGIN_MANAGER_API_GET("draw_2d_api", &draw_2d_api);
    // if (ret < 0)
    // {
    //     return ret;
    // }

    WindowApiCreateWindowOptions create_window_options = {
        .window_name = "My app",
    };
    ret = window_api->create_window(window_api->context, &create_window_options);

    ret = gui_application_api->run(gui_application_api->context);

    TODO("Move this to window api end");
    CoUninitialize();
    return 0;
}