add_library(input_default)

target_compile_options(input_default PRIVATE ${MY_STRICT_WARNINGS})

target_sources(input_default
    PRIVATE
        input_default_register.c
        input_default.c
    
    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(input_default
    PRIVATE
        plugin_manager_include
)

set(PLUGIN_MANAGER_DIR_PATH "C:/Users/Blue-WaveMathijsFran/Documents/GitHub/plugin-framework/lib/plugin_manager_bootloader")

include("${PLUGIN_MANAGER_DIR_PATH}/cmake/PluginSDK.cmake")
plugin_generate_register_inc(input_default
    TOOLS_DIR_PATH "${PLUGIN_MANAGER_DIR_PATH}/tools"
    PYTHON_SCRIPTS_PATH "${PLUGIN_MANAGER_DIR_PATH}/cmake/scripts"
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)