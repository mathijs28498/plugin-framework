add_library(test_plugin_2 SHARED)

target_sources(test_plugin_2
    PRIVATE
        test_plugin_2.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(test_plugin_2
    PRIVATE
        plugin_manager_common
)