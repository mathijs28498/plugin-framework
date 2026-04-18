add_executable(program main.c)

target_compile_options(program PRIVATE ${MY_STRICT_WARNINGS})

if(WIN32 AND PLUGIN_MANAGER_GUI)
    target_link_options(program PRIVATE "/INCLUDE:wWinMain")
    set_target_properties(program PROPERTIES WIN32_EXECUTABLE ON)
endif()

message("TODO: Add other OS support")

target_link_libraries(program
    PRIVATE
    plugin_manager_include
    plugin_manager_bootloader
)
