find_package(Vulkan REQUIRED)

# add_executable(Program main_no_win32.c)

add_executable(Program WIN32 main.c)

if (WIN32)
    set_target_properties(Program PROPERTIES
        WIN32_EXECUTABLE ON
    )
    target_compile_definitions(Program PRIVATE UNICODE _UNICODE COBJMACROS)
endif()
# TODO: Add other OS support

# set_target_properties(Program
#     PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
# )

target_link_libraries(Program
    PRIVATE
        Vulkan::Vulkan
        Cglm
        Renderer
)

add_subdirectory(renderer)