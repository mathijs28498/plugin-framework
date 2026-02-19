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

    // ret = PLUGIN_MANAGER_API_ADD("logic_api", "ecs_logic_api");
    ret = PLUGIN_MANAGER_API_ADD("gui_application_api", NULL);
    ret = PLUGIN_MANAGER_API_LOAD();
    if (ret < 0)
    {
        return -1;
    }

    WindowApi *window_api;
    GuiApplicationApi *gui_application_api;
    // LogicApi *logic_api;
    PLUGIN_MANAGER_API_GET("window_api", &window_api);
    PLUGIN_MANAGER_API_GET("gui_application_api", &gui_application_api);
    // PLUGIN_MANAGER_API_GET("logic_api", &logic_api);

    // logic_api->ecs_specific_function(logic_api->context);

    WindowApiCreateWindowOptions create_window_options = {
        .window_name = "My app",
    };
    ret = window_api->create_window(window_api->context, &create_window_options);

    ret = gui_application_api->run(gui_application_api->context);

    TODO("Move this to window api end");
    CoUninitialize();
    return 0;
}