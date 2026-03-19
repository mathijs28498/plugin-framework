add_library(plugin_manager_bootloader STATIC)

cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH PARENT_DIR)
cmake_path(GET PARENT_DIR PARENT_PATH TOML_DIR_PATH)

if(WIN32 AND PLUGIN_MANAGER_GUI)
    target_compile_definitions(plugin_manager_bootloader PRIVATE UNICODE _UNICODE COBJMACROS)
endif()

message("TODO: Put these back")

include(cmake/PluginManagerBootloaderGenerate.cmake)

plugin_manager_bootloader_setup(plugin_manager_bootloader
    PYTHON_SCRIPTS_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/scripts"
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${TOML_DIR_PATH}")

target_include_directories(plugin_manager_bootloader
    PRIVATE
    "${GEN_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}"
)

target_sources(plugin_manager_bootloader
    PRIVATE

    plugin_manager_bootloader.c
    plugin_manager_bootloader_main.c
    # plugin_metadata.c
    # plugin_registry.c
)

target_link_libraries(plugin_manager_bootloader
    PRIVATE
    plugin_manager_include
    plugin_manager_default
)

message("TODO: Check if this has to go inside the plugin_manager_setup")

if(BUILD_SHARED_LIBS)
    target_compile_definitions(plugin_manager_bootloader PRIVATE PLUGIN_BUILD_SHARED)
endif()