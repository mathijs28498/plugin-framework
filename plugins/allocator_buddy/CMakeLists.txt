add_library(allocator_buddy)

target_compile_options(allocator_buddy PRIVATE ${MY_STRICT_WARNINGS})

target_sources(allocator_buddy
    PRIVATE
    allocator_buddy_register.c
    allocator_buddy.c

    PUBLIC
    FILE_SET HEADERS
)

target_link_libraries(allocator_buddy
    PRIVATE
    plugin_sdk
)

include("pluginSDK")
plugin_generate_register_inc(allocator_buddy
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)