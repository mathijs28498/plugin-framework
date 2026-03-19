add_library(window_win32)

target_sources(window_win32
    PRIVATE
    window_win32.c
    window_win32_register.c
    window_win32_window_events.c
    window_win32_key_converter.c

    PUBLIC
    FILE_SET HEADERS
)

target_compile_definitions(window_win32 PRIVATE UNICODE _UNICODE COBJMACROS)

target_link_libraries(window_win32
    PRIVATE
    plugin_manager_include
)

set(PLUGIN_MANAGER_DIR_PATH "C:/Users/Blue-WaveMathijsFran/Documents/GitHub/notepad-c/lib/plugin_manager_bootloader")

include("${PLUGIN_MANAGER_DIR_PATH}/cmake/PluginSDK.cmake")
plugin_generate_register_inc(window_win32
    TOOLS_DIR_PATH "${PLUGIN_MANAGER_DIR_PATH}/tools"
    PYTHON_SCRIPTS_PATH "${PLUGIN_MANAGER_DIR_PATH}/cmake/scripts"
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)