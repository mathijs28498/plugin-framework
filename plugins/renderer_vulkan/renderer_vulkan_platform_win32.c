#include "renderer_vulkan_platform.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_64_BIT_PTR_DEFINES 1
#include <vulkan/vulkan.h>
#include <stdint.h>
#include <assert.h>

#include <plugin_sdk/logger_interface.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_platform_win32, LOG_LEVEL_DEBUG)
#include <plugin_sdk/window_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include "renderer_vulkan_register.h"

int32_t renderer_vulkan_platform_create_surface(RendererContext *context, VkSurfaceKHR *surface)
{
    assert(context != NULL);
    assert(surface != NULL);

    VkResult result;

    WindowInterfaceOSHandles os_handles = window_get_os_handles(context->window);

    VkWin32SurfaceCreateInfoKHR surface_create_info = {0};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hinstance = (HINSTANCE)os_handles.platform_context_handle;
    surface_create_info.hwnd = (HWND)os_handles.window_handle;

    VK_RETURN_IF_ERROR(context->logger, result, vkCreateWin32SurfaceKHR(context->instance, &surface_create_info, NULL, surface),
                       -1, "Failed to create win32 surface: %d", result);

    return 0;
}

void renderer_vulkan_platform_get_required_extensions(const char ***out_extensions)
{
    assert(out_extensions != NULL);
    CREATE_INITIALIZED_ARRAY_WITH_DECL(
        static, const char *, extensions,
        {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        });

    *out_extensions = extensions;
}