

#include <plugin_sdk/plugin_manager/v1/plugin_manager_interface.h>
#include <plugin_sdk/allocator/v1/allocator_interface.h>
#include <plugin_sdk/plugin_utils.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(main, LOG_LEVEL_DEBUG)
#include <plugin_sdk/window/v1/window_interface.h>
#include <plugin_sdk/gui_application/v1/gui_application_interface.h>

#define DATA_AMOUNT 20

typedef struct TestStruct
{
    uint32_t numba;
} TestStruct;

int32_t plugin_manager_bootloader_main(PluginManagerInterface *plugin_manager)
{
    int32_t ret;

    LoggerInterface *logger;
    ret = PLUGIN_MANAGER_GET_SINGLETON(plugin_manager, "logger", &logger);
    if (ret < 0)
    {
        return ret;
    }

    LOG_WRN_TRACE(logger, "This works baby!");

    AllocatorInterface *allocator;
    RETURN_IF_ERROR(logger, ret, PLUGIN_MANAGER_GET_SINGLETON(plugin_manager, "allocator", &allocator),
                    "Failed to get allocator plugin: %d", ret);

    TestStruct *data;
    (void)data;

    void *test;
    AllocatorAllocationHandle alloc_handle0;
    allocator_alloc(allocator, 20, &alloc_handle0, &test);
    AllocatorAllocationHandle alloc_handle1;
    allocator_alloc(allocator, 255, &alloc_handle1, &test);
    AllocatorAllocationHandle alloc_handle2;
    allocator_alloc(allocator, 511, &alloc_handle2, &test);
    AllocatorAllocationHandle alloc_handle3;
    allocator_alloc(allocator, 256, &alloc_handle3, &test);

    // allocator_alloc(allocator, 511, &alloc_handle,&test);

    // GuiApplicationInterface *gui_application;
    // RETURN_IF_ERROR(logger, ret, PLUGIN_MANAGER_GET_SINGLETON(plugin_manager, "gui_application", &gui_application),
    //                 "Failed to get plugin_manager plugin: %d", ret);

    // WindowInterfaceCreateWindowOptions create_window_options = {
    //     .window_name = "My app",
    // };
    // RETURN_IF_ERROR(logger, ret, gui_application_setup(gui_application, &create_window_options),
    //                 "Failed to setup gui application: %d", ret);
    // RETURN_IF_ERROR(logger, ret, gui_application_run(gui_application),
    //                 "Failed to run gui application: %d", ret);
    return 0;
}