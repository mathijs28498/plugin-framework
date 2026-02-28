add_library(draw_2d_default)

target_sources(draw_2d_default
    PRIVATE
        draw_2d_default.c
        draw_2d_default_register.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(draw_2d_default
    PRIVATE
        plugin_manager_include
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(draw_2d_default PRIVATE PLUGIN_BUILD_SHARED)
endif()
target_compile_definitions(draw_2d_default PRIVATE PLUGIN_INTERFACE_NAME=draw_2d)
