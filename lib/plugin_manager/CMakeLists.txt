add_library(plugin_manager)

include(PluginFramework.cmake)

cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH PARENT_DIR)
cmake_path(GET PARENT_DIR PARENT_PATH JSON_DIR)

set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

plugin_manager_load_plugins("${JSON_DIR}" "${GEN_DIR}")
plugin_manager_load_registry("${JSON_DIR}" "${GEN_DIR}")

target_include_directories(plugin_manager 
    PRIVATE 
        "${GEN_DIR}"
        "${CMAKE_CURRENT_LIST_DIR}"
)

target_sources(plugin_manager
    PRIVATE
        file_io.c
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