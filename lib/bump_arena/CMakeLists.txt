add_library(bump_arena)

target_compile_options(bump_arena PRIVATE ${MY_STRICT_WARNINGS})

target_sources(bump_arena
    PRIVATE
    bump_arena.c

    PUBLIC
    FILE_SET HEADERS
)

target_link_libraries(bump_arena
    PRIVATE
    plugin_sdk
)