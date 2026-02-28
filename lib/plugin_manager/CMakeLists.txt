add_library(plugin_manager STATIC)


cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH PARENT_DIR)
cmake_path(GET PARENT_DIR PARENT_PATH JSON_DIR)

set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

include(PluginManagerLoadPlugins.cmake)
plugin_manager_load_plugins(plugin_manager "${JSON_DIR}" "${GEN_DIR}")
include(PluginManagerLoadRegistry.cmake)
plugin_manager_load_registry(plugin_manager "${JSON_DIR}" "${GEN_DIR}")

target_include_directories(plugin_manager 
    PRIVATE 
        "${GEN_DIR}"
        "${CMAKE_CURRENT_LIST_DIR}"
)

target_sources(plugin_manager
    PRIVATE
        plugin_manager_loader.c
        plugin_manager.c

    # PUBLIC
    #     FILE_SET HEADERS
)

target_link_libraries(plugin_manager
    PRIVATE
        plugin_manager_include
        cjson
        logger_console
        environment_default
)


add_subdirectory(internal_plugins)