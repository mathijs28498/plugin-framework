add_library(arena_allocator_default)

target_compile_options(arena_allocator_default PRIVATE ${MY_STRICT_WARNINGS})

target_sources(arena_allocator_default
    PRIVATE
    arena_allocator_default_register.c
    arena_allocator_default.c

    PUBLIC
    FILE_SET HEADERS
)

target_link_libraries(arena_allocator_default
    PRIVATE
    plugin_sdk
)

include("pluginSDK")
plugin_generate_register_inc(arena_allocator_default
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)