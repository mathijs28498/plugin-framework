add_library(RendererVk)

target_sources(RendererVk
    PRIVATE
        renderer_vk.c
    
    PUBLIC
        FILE_SET HEADERS
)

find_package(Vulkan)

target_link_libraries(RendererVk
    PRIVATE
        Vulkan::Vulkan
)