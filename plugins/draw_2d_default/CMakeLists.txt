add_library(draw_2d_default)

target_sources(draw_2d_default
    PRIVATE
    draw_2d_default_register.c
    draw_2d_default.c
    PUBLIC
    FILE_SET HEADERS
)

target_link_libraries(draw_2d_default
    PRIVATE
    plugin_manager_include
)

set(PLUGIN_MANAGER_DIR_PATH "C:/Users/Blue-WaveMathijsFran/Documents/GitHub/notepad-c/lib/plugin_manager_bootloader")

include("${PLUGIN_MANAGER_DIR_PATH}/cmake/PluginSDK.cmake")
plugin_generate_register_inc(draw_2d_default
    TOOLS_DIR_PATH "${PLUGIN_MANAGER_DIR_PATH}/tools"
    PYTHON_SCRIPTS_PATH "${PLUGIN_MANAGER_DIR_PATH}/cmake/scripts"
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)