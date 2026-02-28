find_package(Vulkan REQUIRED)

add_executable(program main.c)

if (WIN32)
    set_target_properties(program PROPERTIES
        WIN32_EXECUTABLE ON
    )
    target_compile_definitions(program PRIVATE UNICODE _UNICODE COBJMACROS)
endif()

message("TODO: Add other OS support")

target_link_libraries(program
    PRIVATE
        plugin_manager_include
        plugin_manager
)
