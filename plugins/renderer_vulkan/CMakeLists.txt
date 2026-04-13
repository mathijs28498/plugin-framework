find_package(Vulkan REQUIRED)

add_library(renderer_vulkan)

target_sources(renderer_vulkan
    PRIVATE
    renderer_vulkan_register.c
    renderer_vulkan_bootstrap.c
    renderer_vulkan_utils.c
    renderer_vulkan_start.c
    renderer_vulkan_render.c
    renderer_vulkan.c

    PUBLIC
    FILE_SET HEADERS
)

message("TODO: Add other targets")

if(WIN32)
    target_sources(renderer_vulkan PRIVATE renderer_vulkan_platform_win32.c)
endif()

target_link_libraries(renderer_vulkan
    PRIVATE
    plugin_manager_include
    Vulkan::Vulkan
)

set(PLUGIN_MANAGER_DIR_PATH "C:/Users/Blue-WaveMathijsFran/Documents/GitHub/notepad-c/lib/plugin_manager_bootloader")

include("${PLUGIN_MANAGER_DIR_PATH}/cmake/PluginSDK.cmake")
plugin_generate_register_inc(renderer_vulkan
    TOOLS_DIR_PATH "${PLUGIN_MANAGER_DIR_PATH}/tools"
    PYTHON_SCRIPTS_PATH "${PLUGIN_MANAGER_DIR_PATH}/cmake/scripts"
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)