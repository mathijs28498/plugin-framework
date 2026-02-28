add_library(draw_default)

target_sources(draw_default
    PRIVATE
        draw_default.c
        draw_default_register.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(draw_default
    PRIVATE
        plugin_manager_include
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(draw_default PRIVATE PLUGIN_BUILD_SHARED)
endif()
target_compile_definitions(draw_default PRIVATE PLUGIN_INTERFACE_NAME=draw)