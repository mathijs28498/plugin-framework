add_library(dynamic_metadata_resolver STATIC)

target_compile_options(dynamic_metadata_resolver PRIVATE ${MY_STRICT_WARNINGS})

target_sources(dynamic_metadata_resolver
    PUBLIC
    FILE_SET HEADERS
)

message("TODO: Make this a generic loader of dynamic plugins, not just metadata")
message("TODO: Add other targets")

if(WIN32)
    target_sources(dynamic_metadata_resolver PRIVATE dynamic_metadata_resolver_win32.c)
    target_compile_definitions(dynamic_metadata_resolver PRIVATE UNICODE _UNICODE COBJMACROS)
endif()

target_link_libraries(dynamic_metadata_resolver
    PRIVATE
    plugin_sdk
)
