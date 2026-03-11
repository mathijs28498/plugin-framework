add_library(plugin_manager STATIC)

cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH PARENT_DIR)
cmake_path(GET PARENT_DIR PARENT_PATH TOML_DIR_PATH)

include(cmake/PluginManagerGenerate.cmake)

plugin_manager_setup("plugin_manager"
    PYTHON_SCRIPTS_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/scripts"
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${TOML_DIR_PATH}")

target_include_directories(plugin_manager
    PRIVATE
    "${GEN_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}"
)

target_sources(plugin_manager
    PRIVATE
    plugin_manager_loader.c
    plugin_manager.c
)

target_link_libraries(plugin_manager
    PRIVATE
    plugin_manager_include

    # cjson
    # logger_console
    # environment_default
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(plugin_manager PRIVATE PLUGIN_BUILD_SHARED)
endif()