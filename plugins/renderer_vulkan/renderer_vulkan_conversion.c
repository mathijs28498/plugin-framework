#include "renderer_vulkan_conversion.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_conversion, LOG_LEVEL_DEBUG)
#include <plugin_sdk/renderer/v1/renderer_interface.h>

#include "renderer_vulkan_register.h"
#include "renderer_vulkan.h"

VkImageSubresourceRange rv_image_subresource_range(VkImageAspectFlags aspect_mask)
{
    return (VkImageSubresourceRange){
        .aspectMask = aspect_mask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };
}

VkSemaphoreSubmitInfo rv_create_semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore)
{
    return (VkSemaphoreSubmitInfo){
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = semaphore,
        .stageMask = stage_mask,
        .deviceIndex = 0,
        .value = 1,
    };
}

VkCommandBufferSubmitInfo rv_create_command_buffer_submit_info(VkCommandBuffer cmd)
{
    return (VkCommandBufferSubmitInfo){
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
        .deviceMask = 0,
    };
}

VkSubmitInfo2 rv_create_submit_info(VkCommandBufferSubmitInfo *cmd, VkSemaphoreSubmitInfo *signal_semaphore_info, VkSemaphoreSubmitInfo *wait_semaphore_info)
{
    return (VkSubmitInfo2){
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,

        .waitSemaphoreInfoCount = wait_semaphore_info == NULL ? 0 : 1,
        .pWaitSemaphoreInfos = wait_semaphore_info,

        .signalSemaphoreInfoCount = signal_semaphore_info == NULL ? 0 : 1,
        .pSignalSemaphoreInfos = signal_semaphore_info,

        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = cmd,
    };
}

VkDescriptorType rv_resource_type_to_vk_descriptor_type(RendererResourceType resource_type)
{
    switch (resource_type)
    {
    case RENDERER_RESOURCE_TYPE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
        break;
    case RENDERER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
    case RENDERER_RESOURCE_TYPE_SAMPLED_IMAGE:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        break;
    case RENDERER_RESOURCE_TYPE_STORAGE_IMAGE:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        break;

    default:
        UNREACHABLE();
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        break;
    }
}

VkShaderStageFlags rv_shader_stage_to_vk_shader_stage(RendererShaderStageFlags flags)
{
    VkShaderStageFlags vk_flags = 0;

    if (flags & RENDERER_SHADER_STAGE_VERTEX_BIT)
    {
        vk_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (flags & RENDERER_SHADER_STAGE_FRAGMENT_BIT)
    {
        vk_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if (flags & RENDERER_SHADER_STAGE_COMPUTE_BIT)
    {
        vk_flags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }

    return vk_flags;
}

VkPipelineBindPoint rv_pipeline_type_to_vk_pipeline_bind_point(RendererPipelineType pipeline_type)
{
    switch (pipeline_type)
    {
    case RENDERER_PIPELINE_TYPE_GRAPHICS:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    case RENDERER_PIPELINE_TYPE_COMPUTE:
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    default:
        UNREACHABLE();
        return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }
}

VkImageLayout rv_image_layout_to_vk_image_layout(RendererImageLayout image_layout)
{
    switch (image_layout)
    {
    case RENDERER_IMAGE_LAYOUT_UNDEFINED:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case RENDERER_IMAGE_LAYOUT_GENERAL:
        return VK_IMAGE_LAYOUT_GENERAL;
    case RENDERER_IMAGE_LAYOUT_COLOR_ATTACHMENT:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case RENDERER_IMAGE_LAYOUT_TRANSFER_SRC:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case RENDERER_IMAGE_LAYOUT_TRANSFER_DST:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case RENDERER_IMAGE_LAYOUT_PRESENT_SRC:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:
        UNREACHABLE();
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

VkFormat rv_image_format_to_vk_format(RendererImageFormat format)
{
    switch (format)
    {
    case RENDERER_IMAGE_FORMAT_UNDEFINED:
        return VK_FORMAT_UNDEFINED;
    case RENDERER_IMAGE_FORMAT_R8G8B8A8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case RENDERER_IMAGE_FORMAT_R8G8B8A8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case RENDERER_IMAGE_FORMAT_R16G16B16A16_SFLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case RENDERER_IMAGE_FORMAT_R32G32B32A32_SFLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case RENDERER_IMAGE_FORMAT_D32_SFLOAT:
        return VK_FORMAT_D32_SFLOAT;
    case RENDERER_IMAGE_FORMAT_D24_UNORM_S8_UINT:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    default:
        UNREACHABLE();
        return VK_FORMAT_UNDEFINED;
    }
}
VkBufferUsageFlags rv_buffer_usage_to_vk_buffer_usage(RendererBufferUsageFlags renderer_flags)
{
    VkBufferUsageFlags vk_flags = 0;
    if (renderer_flags & RENDERER_BUFFER_USAGE_TRANSFER_SRC_BIT)
    {
        vk_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (renderer_flags & RENDERER_BUFFER_USAGE_TRANSFER_DST_BIT)
    {
        vk_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if (renderer_flags & RENDERER_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)
    {
        vk_flags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (renderer_flags & RENDERER_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
    {
        vk_flags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }
    if (renderer_flags & RENDERER_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    {
        vk_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (renderer_flags & RENDERER_BUFFER_USAGE_STORAGE_BUFFER_BIT)
    {
        vk_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (renderer_flags & RENDERER_BUFFER_USAGE_INDEX_BUFFER_BIT)
    {
        vk_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (renderer_flags & RENDERER_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    {
        vk_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (renderer_flags & RENDERER_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
    {
        vk_flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if (renderer_flags & RENDERER_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        vk_flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }
    return vk_flags;
}

VmaMemoryUsage rv_memory_usage_to_vma_memory_usage(RendererMemoryUsage renderer_memory_usage)
{
    switch (renderer_memory_usage)
    {
    case RENDERER_MEMORY_USAGE_GPU_ONLY:
        return VMA_MEMORY_USAGE_GPU_ONLY;
    case RENDERER_MEMORY_USAGE_CPU_ONLY:
        return VMA_MEMORY_USAGE_CPU_ONLY;
    default:
        UNREACHABLE();
        return 0;
    }
}

VkImageUsageFlags rv_image_usage_to_vk_image_usage(RendererImageUsageFlags flags)
{
    VkImageUsageFlags vk_flags = 0;
    if (flags & RENDERER_IMAGE_USAGE_TRANSFER_SRC_BIT)
        vk_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (flags & RENDERER_IMAGE_USAGE_TRANSFER_DST_BIT)
        vk_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (flags & RENDERER_IMAGE_USAGE_SAMPLED_BIT)
        vk_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (flags & RENDERER_IMAGE_USAGE_STORAGE_BIT)
        vk_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (flags & RENDERER_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        vk_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (flags & RENDERER_IMAGE_USAGE_DEPTH_ATTACHMENT_BIT)
        vk_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    return vk_flags;
}

VkImageAspectFlags rv_vk_format_to_image_aspect(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D16_UNORM:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

VkMemoryPropertyFlags rv_image_memory_usage_to_vk_memory_usage(RendererMemoryUsage memory_usage)
{
    switch (memory_usage)
    {
    case RENDERER_MEMORY_USAGE_GPU_ONLY:
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    default:
        UNREACHABLE();
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
}

VkImageLayout rv_attachment_type_to_vk_image_layout(RendererAttachmentType attachment_type)
{
    switch (attachment_type)
    {
    case RENDERER_ATTACHMENT_TYPE_COLOR:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case RENDERER_ATTACHMENT_TYPE_DEPTH:
        return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    default:
        UNREACHABLE();
        return 0;
    };
}

VkAttachmentLoadOp rv_attachment_load_op_to_vk_attachment_load_op(RendererAttachmentLoadOp renderer_attachment_load_op)
{
    switch (renderer_attachment_load_op)
    {
    case RENDERER_ATTACHMENT_LOAD_OP_LOAD:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case RENDERER_ATTACHMENT_LOAD_OP_CLEAR:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case RENDERER_ATTACHMENT_LOAD_OP_DONT_CARE:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    default:
        UNREACHABLE();
        return 0;
    };
}

VkAttachmentStoreOp rv_attachment_store_op_to_vk_attachment_store_op(RendererAttachmentStoreOp renderer_attachment_store_op)
{
    switch (renderer_attachment_store_op)
    {
    case RENDERER_ATTACHMENT_STORE_OP_STORE:
        return VK_ATTACHMENT_STORE_OP_STORE;
    case RENDERER_ATTACHMENT_STORE_OP_DONT_CARE:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    default:
        UNREACHABLE();
        return 0;
    };
}

VkRenderingAttachmentInfo rv_attachment_info_to_vk_attachment_info(const RendererAttachmentInfo *renderer_attachment_info, RV_AllocatedImage *image, RendererAttachmentType attachment_type)
{
    assert(renderer_attachment_info != NULL);
    assert(image != NULL);

    VkClearValue clear_value =
        attachment_type == RENDERER_ATTACHMENT_TYPE_COLOR
            ? (VkClearValue){
                  .color = {
                      renderer_attachment_info->clear_value.color.r,
                      renderer_attachment_info->clear_value.color.g,
                      renderer_attachment_info->clear_value.color.b,
                      renderer_attachment_info->clear_value.color.a,
                  }}
            : (VkClearValue){.depthStencil.depth = renderer_attachment_info->clear_value.depth};

    return (VkRenderingAttachmentInfo){
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = image->image_view,
        .imageLayout = rv_attachment_type_to_vk_image_layout(attachment_type),
        .loadOp = rv_attachment_load_op_to_vk_attachment_load_op(renderer_attachment_info->load_op),
        .storeOp = rv_attachment_store_op_to_vk_attachment_store_op(renderer_attachment_info->store_op),
        .clearValue = clear_value,
    };
}

VkExtent3D rv_renderer_extent_3d_to_vk_extent_3d(const RendererExtent3D *renderer_extent)
{
    return (VkExtent3D){.width = renderer_extent->width,
                        .height = renderer_extent->height,
                        .depth = renderer_extent->depth};
}

RendererExtent3D rv_vk_extent_3d_to_renderer_3d(const VkExtent3D *renderer_extent)
{
    return (RendererExtent3D){.width = renderer_extent->width,
                              .height = renderer_extent->height,
                              .depth = renderer_extent->depth};
}

VkExtent2D rv_renderer_extent_2d_to_vk_extent_2d(const RendererExtent2D *renderer_extent)
{
    return (VkExtent2D){.width = renderer_extent->width,
                        .height = renderer_extent->height};
}

RendererExtent2D rv_vk_extent_2d_to_renderer_2d(const VkExtent2D *renderer_extent)
{
    return (RendererExtent2D){.width = renderer_extent->width,
                              .height = renderer_extent->height};
}

#define rv_vk_extent_3d_to_renderer_3d(renderer_extent) \
    (RendererExtent3D) { .width = (vk_extent).width, .height = (vk_extent).height, .depth = (vk_extent).depth }

#define rv_renderer_extent_2d_to_vk_extent_2d(renderer_extent) \
    (VkExtent2D) { .width = (renderer_extent).width, .height = (renderer_extent).height }

#define rv_vk_extent_2d_to_renderer_extent_2d(vk_extent) \
    (RendererExtent3D) { .width = (vk_extent).width, .height = (vk_extent).height }

VkPrimitiveTopology rv_topology_to_vk_topology(RendererPrimitiveTopology topology)
{
    switch (topology)
    {
    case RENDERER_PRIMITIVE_TOPOLOGY_POINT_LIST:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case RENDERER_PRIMITIVE_TOPOLOGY_LINE_LIST:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case RENDERER_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case RENDERER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case RENDERER_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case RENDERER_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    default:
        UNREACHABLE();
        return 0;
    }
}

VkPolygonMode rv_fill_mode_to_vk_polygon_mode(RendererFillMode fill_mode)
{
    switch (fill_mode)
    {
    case RENDERER_FILL_MODE_SOLID:
        return VK_POLYGON_MODE_FILL;
    case RENDERER_FILL_MODE_WIREFRAME:
        return VK_POLYGON_MODE_LINE;
    case RENDERER_FILL_MODE_POINT:
        return VK_POLYGON_MODE_POINT;
    default:
        UNREACHABLE();
        return 0;
    }
}

TODO("Improve this method, maybe enum in stead of bits or something?")
VkCullModeFlagBits rv_cull_mode_to_vk_cull_mode(RendererCullModeFlagBits cull_mode)
{
    switch (cull_mode)
    {
    case RENDERER_CULL_MODE_NONE:
        return VK_CULL_MODE_NONE;
    case RENDERER_CULL_MODE_FRONT_BIT:
        return VK_CULL_MODE_FRONT_BIT;
    case RENDERER_CULL_MODE_BACK_BIT:
        return VK_CULL_MODE_BACK_BIT;
    case RENDERER_CULL_MODE_FRONT_AND_BACK:
        return VK_CULL_MODE_FRONT_AND_BACK;
    default:
        UNREACHABLE();
        return 0;
    }
}

VkFrontFace rv_front_face_to_vk_front_face(RendererFrontFace front_face)
{
    switch (front_face)
    {
    case RENDERER_FRONT_FACE_COUNTER_CLOCKWISE:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case RENDERER_FRONT_FACE_CLOCKWISE:
        return VK_FRONT_FACE_CLOCKWISE;
    default:
        UNREACHABLE();
        return 0;
    }
}
