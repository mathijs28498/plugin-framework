find_package(Vulkan REQUIRED)

# add_executable(Program main_no_win32.c)

add_executable(program main.c)

if (WIN32)
    # set_target_properties(Program PROPERTIES
    #     WIN32_EXECUTABLE ON
    # )
    # target_compile_definitions(Program PRIVATE UNICODE _UNICODE COBJMACROS)
endif()
# TODO: Add other OS support

target_link_libraries(program
    PRIVATE
        # Vulkan::Vulkan
        # Cglm
        # Renderer
        # ProjectCommon
        # TestPlugin
        plugin_common
        plugin_loader
        test_lib
        test_yes
)

# add_subdirectory(renderer)
add_subdirectory(plugin_loader)