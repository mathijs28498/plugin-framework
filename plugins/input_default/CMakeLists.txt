add_library(input_default)

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

if (BUILD_SHARED_LIBS)
    target_compile_definitions(input_default PRIVATE PLUGIN_BUILD_SHARED)
endif()
target_compile_definitions(input_default PRIVATE PLUGIN_INTERFACE_NAME=input)