add_library(Renderer)

target_sources(Renderer
    PRIVATE
        renderer.c
    PUBLIC 
        FILE_SET HEADERS
)

target_link_libraries(Renderer
    PRIVATE
        RendererVk
)


add_subdirectory(renderer_vk)
