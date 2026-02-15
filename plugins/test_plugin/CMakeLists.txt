add_library(test_plugin SHARED)

target_sources(test_plugin
    PRIVATE
        test_plugin.c
    
    PUBLIC
        FILE_SET HEADERS
        FILES
            test_plugin.h
)

target_link_libraries(test_plugin
    PRIVATE
        plugin_manager_common
)