add_library(plugin_manager_bootloader STATIC)

target_compile_options(plugin_manager_bootloader PRIVATE ${MY_STRICT_WARNINGS})

if(WIN32)
    target_compile_definitions(plugin_manager_bootloader PRIVATE UNICODE _UNICODE COBJMACROS)
endif()

get_filename_component(TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
get_filename_component(PLUGIN_SDK_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/../plugin_sdk" ABSOLUTE)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/build_tools/cmake")

include(PluginManagerBootloaderGenerate)

plugin_manager_bootloader_setup(plugin_manager_bootloader
    PYTHON_SCRIPTS_PATH "${CMAKE_CURRENT_LIST_DIR}/build_tools/scripts"
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${TOML_DIR_PATH}"
    PLUGIN_SDK_ROOT_PATH "${PLUGIN_SDK_ROOT_PATH}"
)

target_include_directories(plugin_manager_bootloader
    PRIVATE
    "${GEN_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}"
)

target_sources(plugin_manager_bootloader
    PRIVATE
    plugin_manager_bootloader.c
    plugin_manager_bootloader_main.c
)

message("TODO: Add other targets")

target_link_libraries(plugin_manager_bootloader
    PRIVATE
    plugin_sdk
    dynamic_metadata_resolver
)

message("TODO: Check if this has to go inside the plugin_manager_setup")

if(BUILD_SHARED_LIBS)
    target_compile_definitions(plugin_manager_bootloader PRIVATE PLUGIN_BUILD_SHARED)
endif()