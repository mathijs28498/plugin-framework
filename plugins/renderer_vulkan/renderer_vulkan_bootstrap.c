#include "renderer_vulkan_bootstrap.h"

#include <plugin_sdk/plugin_utils.h>

#include <stdint.h>
#include <stdbool.h>

#define VK_USE_64_BIT_PTR_DEFINES 1
#include <vulkan/vulkan.h>
#include <string.h>
#include <assert.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
// LOGGER_INTERFACE_REGISTER(renderer_vulkan_start, LOG_LEVEL_DEBUG)
LOGGER_INTERFACE_REGISTER(renderer_vulkan_start, LOG_LEVEL_WARNING)
#include <plugin_sdk/window/v1/window_interface.h>

#include "renderer_vulkan_register.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan.h"
#include "renderer_vulkan_platform.h"

TODO("Change this array based on target (eg when using 1.1 for mobile, remove 1.3 features)")
TODO("Or make them optional if theyre not there and propagate the options")
CREATE_INITIALIZED_ARRAY_WITH_DECL(
    static, const char *, plugin_required_extensions,
    {VK_EXT_DEBUG_UTILS_EXTENSION_NAME});

CREATE_INITIALIZED_ARRAY_WITH_DECL(
    static, const char *, required_physical_device_extensions,
    {VK_KHR_SWAPCHAIN_EXTENSION_NAME});

static VkPhysicalDeviceVulkan12Features required_features_12 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
    .bufferDeviceAddress = VK_TRUE,
    .descriptorIndexing = VK_TRUE,
};

static VkPhysicalDeviceVulkan13Features required_features_13 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    .pNext = &required_features_12,
    .dynamicRendering = VK_TRUE,
    .synchronization2 = VK_TRUE,
};

#if IS_DEBUG
TODO("Make this a setting for the plugin")
CREATE_INITIALIZED_ARRAY_WITH_DECL(
    static, const char *, plugin_required_validation_layers,
    {"VK_LAYER_KHRONOS_validation"});
#endif // #if IS_DEBUG

TODO("Add resource cleanup on partial failure, on error do the");
TODO("Only add required arguments to functions (once deps is a separate struct inside the context)")

TODO("Make a better version for certain parts that reoccure")
/*
TODO: Improve
- [ ] Getting capabilities and looping over them
    - [ ] Dont use the heap or static arrays, use an arena allocator
    - [ ] Figure out how to get access to an arena allocator
- [ ] Settings for chosing all the different parts (like extensions, present_modes etc)
    - [ ] Make sure all settings are in 1 place
    - [ ] Make the user able to pick these settings with sane defaults
    - [ ] Maybe make presets like performance vs energy saver (determined by the plugin)
    - [ ] should settings be interface or plugin level?

- [ ] Stop using the context for all the functions, use the actual interfaces needed for helper functions
*/

#define MAX_SURFACE_FORMATS_LEN 64
#define MAX_PRESENT_MODES_LEN 64

typedef struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    ARRAY_FIELD(VkSurfaceFormatKHR, surface_formats, MAX_SURFACE_FORMATS_LEN);
    ARRAY_FIELD(VkPresentModeKHR, present_modes, MAX_PRESENT_MODES_LEN);
} SwapchainSupportDetails;

TODO("Make this file the bootstrap, only showing a bootstrap function")
TODO("")

#if IS_DEBUG
bool check_validation_layer_support(const char **validation_layers)
{
    assert(validation_layers != NULL);

    uint32_t layers_len;
    vkEnumerateInstanceLayerProperties(&layers_len, NULL);
    assert(layers_len <= MAX_INSTANCE_LAYER_PROPERTIES_LEN);

    CREATE_ARRAY_WITH_LEN(VkLayerProperties, layers, MAX_INSTANCE_LAYER_PROPERTIES_LEN, layers_len);
    vkEnumerateInstanceLayerProperties(&layers_len, layers);

    for (size_t i = 0; i < GET_ARRAY_LENGTH(validation_layers); i++)
    {
        const char *requested_layer = validation_layers[i];
        bool layer_found = false;

        for (uint32_t j = 0; j < layers_len; j++)
        {
            VkLayerProperties layer = layers[j];
            if (strcmp(requested_layer, layer.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }

        if (!layer_found)
        {
            return false;
        }
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *p_user_data)
{
    assert(p_user_data != NULL);
    (void)message_type;
    RendererContext *context = (RendererContext *)p_user_data;

    switch (message_severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_DBG(context->deps.logger, "validation layer: %s", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG_INF(context->deps.logger, "validation layer: %s", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_WRN(context->deps.logger, "validation layer: %s", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_ERR(context->deps.logger, "validation layer: %s", callback_data->pMessage);
        break;
    }

    return VK_FALSE;
}

void populate_debug_messenger_create_info(RendererContext *context, VkDebugUtilsMessengerCreateInfoEXT *debug_messenger_create_info)
{
    assert(context != NULL);
    assert(debug_messenger_create_info != NULL);

    debug_messenger_create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_messenger_create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    debug_messenger_create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    debug_messenger_create_info->pfnUserCallback = debug_callback;
    debug_messenger_create_info->pUserData = (void *)context;
}

int32_t setup_debug_messenger(RendererContext *context)
{
    assert(context != NULL);
    int32_t ret;

    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = {0};
    populate_debug_messenger_create_info(context, &debug_messenger_create_info);

    PFN_vkCreateDebugUtilsMessengerEXT create_func;
    RETURN_IF_ERROR(
        context->deps.logger, ret,
        vk_get_instance_proc(context->deps.logger, context->instance, "vkCreateDebugUtilsMessengerEXT", (vk_func_void_void *)&create_func),
        "Could not get instance proc: %d", ret);

    create_func(context->instance, &debug_messenger_create_info, NULL, &context->debug_messenger);

    PFN_vkDestroyDebugUtilsMessengerEXT destroy_func;
    ret = vk_get_instance_proc(context->deps.logger, context->instance, "vkDestroyDebugUtilsMessengerEXT", (vk_func_void_void *)&destroy_func);
    if (ret < 0)
    {
        LOG_ERR_TRACE(context->deps.logger, "Could not get instance proc: %d", ret);
        return ret;
    }

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->main_destroy_queue, destroy_func, context->instance, context->debug_messenger, NULL),
                    "Failed to push instance destroy data to destroy queue: %d", ret);

    return 0;
}

#endif // #if IS_DEBUG

int32_t create_instance(RendererContext *context)
{
    assert(context != NULL);

    TODO("Use vkEnumerateInstanceVerison to find available versions and chose version accordingly")
    TODO("There are different versions for instance and device level version")
    // https://docs.vulkan.org/guide/latest/versions.html
    // https://docs.vulkan.org/spec/latest/chapters/extensions.html#extendingvulkan-coreversions-queryingversionsupport
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

    CREATE_ARRAY(const char *, required_extensions, MAX_EXTENSIONS_LEN);
    ARRAY_PUSH_ARRAY(required_extensions, plugin_required_extensions,
                     {
                         LOG_ERR_TRACE(context->deps.logger, "Unable to add required extension, reached max capacity");
                         return -1;
                     });

    const char **platform_required_extensions = NULL;
    renderer_vulkan_platform_get_required_extensions(&platform_required_extensions);
    assert(platform_required_extensions != NULL);
    TODO("Make the required_extensions allocated based on size of both extensions by arena allocator");

    ARRAY_PUSH_MULTI_CHECKED(
        required_extensions, platform_required_extensions, GET_ARRAY_LENGTH(platform_required_extensions),
        {
            LOG_ERR_TRACE(context->deps.logger, "Unable to add platform extensions to required extensions, reached max capacity");
            return -1;
        });

    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = (uint32_t)GET_ARRAY_LENGTH(required_extensions),
        .ppEnabledExtensionNames = required_extensions,
    };

    // Validation layers
#if IS_DEBUG
    RETURN_IF_FALSE(context->deps.logger,
                    check_validation_layer_support(plugin_required_validation_layers),
                    -1, "Validation layers not supported");

    instance_create_info.ppEnabledLayerNames = plugin_required_validation_layers;
    instance_create_info.enabledLayerCount = (uint32_t)GET_ARRAY_LENGTH(plugin_required_validation_layers);

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
    populate_debug_messenger_create_info(context, &debug_create_info);
    instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;

#else
    instance_create_info.enabledLayerCount = 0;
#endif // #if IS_DEBUG

    VkResult result;
    int32_t ret;

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkCreateInstance(&instance_create_info, NULL, &context->instance),
                       -1, "Failed to create vulkan instance!");

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_2(context->deps.logger, &context->main_destroy_queue, vkDestroyInstance, context->instance, NULL),
                    "Failed to push instance destroy data to destroy queue: %d", ret);

    return 0;
}

int32_t create_surface(RendererContext *context)
{
    assert(context != NULL);
    int32_t ret;
    RETURN_IF_ERROR(context->deps.logger, ret, renderer_vulkan_platform_create_surface(context, &context->surface),
                    "Failed to create platform surface: %d", ret);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->main_destroy_queue, vkDestroySurfaceKHR, context->instance, context->surface, NULL),
                    "Failed to push surface destroy data to destroy queue: %d", ret);
    return 0;
}

int32_t find_queue_families(RendererContext *context, VkPhysicalDevice physical_device, QueueFamilyIndices *out_queue_family_indices)
{
    assert(context != NULL);
    assert(physical_device != VK_NULL_HANDLE);
    assert(out_queue_family_indices != NULL);

    VkResult result;

    uint32_t queue_families_len = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_len, NULL);

    assert(queue_families_len > 0);
    RETURN_IF_FALSE(context->deps.logger, queue_families_len > 0, -1, "No queue families found for physical device");

    CREATE_ARRAY_WITH_LEN(VkQueueFamilyProperties, queue_families, MAX_QUEUE_FAMILIES_LEN, queue_families_len);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_len, queue_families);

    for (uint32_t i = 0; i < queue_families_len; i++)
    {
        VkQueueFamilyProperties queue_family = queue_families[i];
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            out_queue_family_indices->has_graphics_family = true;
            out_queue_family_indices->graphics_family = i;
        }

        VkBool32 present_support;
        VK_RETURN_IF_ERROR(context->deps.logger, result, vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, context->surface, &present_support),
                           -1, "Unable to get physical device surface support: %d", result);

        if (present_support)
        {
            out_queue_family_indices->has_present_family = true;
            out_queue_family_indices->present_family = i;
        }
    }

    return 0;
}

bool queue_family_indices_is_complete(const QueueFamilyIndices *queue_family_indices)
{
    assert(queue_family_indices != NULL);
    return queue_family_indices->has_graphics_family && queue_family_indices->has_present_family;
}

int32_t physical_device_extensions_are_supported(RendererContext *context, VkPhysicalDevice physical_device, bool *out_result)
{
    assert(context != NULL);
    assert(physical_device != VK_NULL_HANDLE);
    assert(out_result != NULL);

    uint32_t extensions_len;
    VkResult result;
    *out_result = false;

    TODO("Find a location for this")

    TODO("Create an arena allocator for the bootstrapping");
    VK_RETURN_IF_ERROR(context->deps.logger, result, vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extensions_len, NULL),
                       -1, "Failed to enumerate physical device extensions", result);
    assert(extensions_len > 0);

    assert(MAX_PHYSICAL_DEVICE_EXTENSIONS_LEN >= extensions_len);
    CREATE_ARRAY_WITH_LEN(VkExtensionProperties, extensions, MAX_PHYSICAL_DEVICE_EXTENSIONS_LEN, extensions_len);
    uint32_t extensions_cap = (uint32_t)GET_ARRAY_CAPACITY(extensions);
    VK_RETURN_IF_ERROR(context->deps.logger, result, vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extensions_cap, extensions),
                       -1, "Failed to enumerate physical device extensions: %d", result);

    for (size_t i = 0; i < GET_ARRAY_LENGTH(required_physical_device_extensions); i++)
    {
        const char *required_extension = required_physical_device_extensions[i];

        bool extension_found = false;
        for (size_t j = 0; j < extensions_len; j++)
        {
            VkExtensionProperties extension = extensions[j];
            if (strcmp(required_extension, extension.extensionName) == 0)
            {
                extension_found = true;
                break;
            }
        }

        if (!extension_found)
        {
            *out_result = false;
            return 0;
        }
    }

    *out_result = true;
    return 0;
}

int32_t query_swapchain_support_details(RendererContext *context, VkPhysicalDevice physical_device, SwapchainSupportDetails *out_swapchain_support_details)
{
    assert(context != NULL);
    assert(physical_device != VK_NULL_HANDLE);
    assert(out_swapchain_support_details != NULL);

    VkResult result;

    VK_RETURN_IF_ERROR(context->deps.logger, result,
                       vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, context->surface, &out_swapchain_support_details->capabilities),
                       -1, "Failed to get device surface capabilities: %d", result);

    uint32_t format_len;
    VK_RETURN_IF_ERROR(context->deps.logger, result, vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, context->surface, &format_len, NULL),
                       -1, "Failed to get physical device formats: %d", result);
    assert(format_len > 0);

    assert(format_len <= GET_ARRAY_CAPACITY(out_swapchain_support_details->surface_formats));
    GET_ARRAY_LENGTH(out_swapchain_support_details->surface_formats) = (size_t)format_len;

    VK_RETURN_IF_ERROR(context->deps.logger, result,
                       vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, context->surface, &format_len, out_swapchain_support_details->surface_formats),
                       -1, "Failed to get physical device formats: %d", result);

    uint32_t present_mode_len;
    VK_RETURN_IF_ERROR(context->deps.logger, result, vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, context->surface, &present_mode_len, NULL),
                       -1, "Failed to get physical device present modes: %d", result);
    assert(present_mode_len > 0);

    assert(present_mode_len <= GET_ARRAY_CAPACITY(out_swapchain_support_details->present_modes));
    GET_ARRAY_LENGTH(out_swapchain_support_details->present_modes) = (size_t)present_mode_len;

    VK_RETURN_IF_ERROR(context->deps.logger, result,
                       vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, context->surface, &present_mode_len, out_swapchain_support_details->present_modes),
                       -1, "Failed to get physical device present modes: %d", result);
    return 0;
}

bool is_swapchain_adequate(const SwapchainSupportDetails *swapchain_support_details)
{
    assert(swapchain_support_details != NULL);
    return GET_ARRAY_LENGTH(swapchain_support_details->surface_formats) > 0 &&
           GET_ARRAY_LENGTH(swapchain_support_details->present_modes) > 0;
}

// This function requires:
//  - the required and supported structs to be the same type
//  - the struct to have a FeatureHeader with sType and pNext
//  - the required and supported structs should be 0 initialized to avoid false comparisons with padding at the end of the struct
bool compare_required_features(const void *required_struct, const void *supported_struct, size_t struct_size)
{
    assert(required_struct != NULL);
    assert(supported_struct != NULL);

    struct FeatureHeader
    {
        VkStructureType sType;
        void *pNext;
    };
    size_t offset = sizeof(struct FeatureHeader);
    assert(struct_size > offset);
    size_t struct_size_to_compare = struct_size - offset;

    const uint32_t *required_bytes = (const uint32_t *)((const uint8_t *)required_struct + offset);
    const uint32_t *supported_bytes = (const uint32_t *)((const uint8_t *)supported_struct + offset);

    for (size_t i = 0; i < struct_size_to_compare / sizeof(uint32_t); i++)
    {
        if ((~required_bytes[i] | supported_bytes[i]) != 0xFFFFFFFFU)
        {
            return false;
        }
    }

    return true;
}

bool physical_device_features_are_supported(RendererContext *context, VkPhysicalDevice physical_device)
{
    assert(context != NULL);
    assert(physical_device != VK_NULL_HANDLE);

    VkPhysicalDeviceVulkan12Features features_12 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
    };

    VkPhysicalDeviceVulkan13Features features_13 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = (void *)&features_12,
    };

    VkPhysicalDeviceFeatures2 device_features_2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = (void *)&features_13,
    };

    vkGetPhysicalDeviceFeatures2(physical_device, &device_features_2);

    // TODO: Add check if the necessary features are supported
    return compare_required_features(&required_features_13, &features_13, sizeof(required_features_13)) &&
           compare_required_features(&required_features_12, &features_12, sizeof(required_features_12));
}

bool is_device_suitable(RendererContext *context, VkPhysicalDevice physical_device)
{
    assert(context != NULL);
    assert(physical_device != VK_NULL_HANDLE);
    int32_t ret;

    QueueFamilyIndices queue_family_indices = {0};
    RETURN_IF_ERROR_RET_VALUE(context->deps.logger, ret, find_queue_families(context, physical_device, &queue_family_indices),
                              false, "Failed to find queue family indices: %d", ret);

    bool extensions_are_supported;
    RETURN_IF_ERROR_RET_VALUE(context->deps.logger, ret, physical_device_extensions_are_supported(context, physical_device, &extensions_are_supported),
                              false, "Failed to check physical device extensions: %d", ret);

    SwapchainSupportDetails swapchain_support_details;
    SET_ARRAY_FIELD_CAPACITY(swapchain_support_details.surface_formats);
    SET_ARRAY_FIELD_CAPACITY(swapchain_support_details.present_modes);

    RETURN_IF_ERROR_RET_VALUE(context->deps.logger, ret, query_swapchain_support_details(context, physical_device, &swapchain_support_details),
                              false, "Failed to query swapchain support details: %d", ret);

    return queue_family_indices_is_complete(&queue_family_indices) &&
           extensions_are_supported &&
           is_swapchain_adequate(&swapchain_support_details) &&
           physical_device_features_are_supported(context, physical_device);
}

int32_t rate_physical_device_suitability(RendererContext *context, VkPhysicalDevice physical_device)
{
    assert(context != NULL);
    assert(physical_device != VK_NULL_HANDLE);
    TODO("Improve this algorithm so that it always choses the right gpu");

    if (!is_device_suitable(context, physical_device))
    {
        return -1;
    }

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);

    if (device_properties.apiVersion < VK_API_VERSION_1_3)
    {
        uint32_t major = VK_API_VERSION_MAJOR(device_properties.apiVersion);
        uint32_t minor = VK_API_VERSION_MINOR(device_properties.apiVersion);

        TODO("Add better logging");
        LOG_WRN_TRACE(context->deps.logger, "Device '%s' supports up to Vulkan %u.%u, Required 1.3", device_properties.deviceName, major, minor);
        return -1;
    }

    int32_t score = 0;
    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    score += device_properties.limits.maxImageDimension2D;

    return score;
}

int32_t pick_physical_device(RendererContext *context)
{
    assert(context != NULL);

    uint32_t physical_devices_len = 0;
    VkResult result;
    VK_RETURN_IF_ERROR(context->deps.logger, result, vkEnumeratePhysicalDevices(context->instance, &physical_devices_len, NULL),
                       -1, "Unable to get physical device count");

    assert(physical_devices_len != 0);
    RETURN_IF_TRUE(context->deps.logger, physical_devices_len == 0,
                   -1, "Cannot find a GPU with vulkan support");

    assert(MAX_PHYSICAL_DEVICES_LEN >= physical_devices_len);
    CREATE_ARRAY_WITH_LEN(VkPhysicalDevice, physical_devices, MAX_PHYSICAL_DEVICES_LEN, physical_devices_len);
    VK_RETURN_IF_ERROR(context->deps.logger, result, vkEnumeratePhysicalDevices(context->instance, &physical_devices_len, physical_devices),
                       -1, "Unable to get physical devices");

    size_t chosen_physical_device_index = 0;
    int32_t max_physical_device_score = -1;
    for (uint32_t i = 0; i < physical_devices_len; i++)
    {
        VkPhysicalDevice physical_device_candidate = physical_devices[i];
        int32_t score = rate_physical_device_suitability(context, physical_device_candidate);
        if (score > -1 && score > max_physical_device_score)
        {
            chosen_physical_device_index = (size_t)i;
            max_physical_device_score = score;
        }
    }

    RETURN_IF_TRUE(context->deps.logger, max_physical_device_score < 0, -1, "No suitable GPU found");

    context->physical_device = physical_devices[chosen_physical_device_index];

    return 0;
}

int32_t push_queue_family_indices_to_create_info_arr(
    RendererContext *context,
    QueueFamilyIndices *queue_family_indices,
    VkDeviceQueueCreateInfo *queue_create_info_arr)
{
    assert(context != NULL);
    assert(queue_family_indices != NULL);
    assert(queue_create_info_arr != NULL);

    float queue_priority = 1;

    CREATE_INITIALIZED_ARRAY(
        uint32_t, queue_family_uint32_indices,
        {
            queue_family_indices->graphics_family,
            queue_family_indices->present_family,
        });

    for (size_t i = 0; i < GET_ARRAY_LENGTH(queue_family_uint32_indices); i++)
    {
        uint32_t family_index = queue_family_uint32_indices[i];
        bool duplicate_index = false;

        for (size_t j = 0; j < i; j++)
        {
            uint32_t prev_family_index = queue_family_uint32_indices[j];
            if (prev_family_index == family_index)
            {
                duplicate_index = true;
                break;
            }
        }

        if (duplicate_index)
        {
            continue;
        }

        VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = family_index,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };

        ARRAY_PUSH_CHECKED(queue_create_info_arr, queue_create_info, {
            LOG_ERR_TRACE(context->deps.logger, "Reached max capacity when adding queue create info");
            return -1;
        });
    }

    return 0;
}

int32_t create_logical_device(RendererContext *context)
{
    assert(context != NULL);

    int32_t ret;
    VkResult result;

    QueueFamilyIndices queue_family_indices = {0};
    RETURN_IF_ERROR(context->deps.logger, ret, find_queue_families(context, context->physical_device, &queue_family_indices),
                    "Error finding queue family indices: %d", ret);
    assert(queue_family_indices_is_complete(&queue_family_indices));
    RETURN_IF_FALSE(context->deps.logger, queue_family_indices_is_complete(&queue_family_indices),
                    -1, "Chosen physical device does not all necessary queue families");

    CREATE_ARRAY(VkDeviceQueueCreateInfo, queue_create_info_arr, MAX_QUEUE_CREATE_INFO_ARR_LEN);

    RETURN_IF_ERROR(context->deps.logger, ret, push_queue_family_indices_to_create_info_arr(context, &queue_family_indices, queue_create_info_arr),
                    "Failed to push family indices to create_info_arr: %d", ret);

    VkPhysicalDeviceFeatures physical_device_features = {0};

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &required_features_13,
        .pQueueCreateInfos = queue_create_info_arr,
        .queueCreateInfoCount = (uint32_t)GET_ARRAY_LENGTH(queue_create_info_arr),
        .pEnabledFeatures = &physical_device_features,

        .enabledExtensionCount = (uint32_t)GET_ARRAY_LENGTH(required_physical_device_extensions),
        .ppEnabledExtensionNames = required_physical_device_extensions,
        // NOTE: We set these to 0 as this was only used in legacy, modern versions do not use it anymore
        //       Add the validation layers if it is ever build for such a legacy system
        .enabledLayerCount = 0,
    };

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkCreateDevice(context->physical_device, &device_create_info, NULL, &context->device),
                       -1, "Failed to create device: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_2(context->deps.logger, &context->main_destroy_queue, vkDestroyDevice, context->device, NULL),
                    "Failed to push device destroy data to destroy queue: %d", ret);

    vkGetDeviceQueue(context->device, queue_family_indices.graphics_family, 0, &context->graphics_queue);
    assert(context->graphics_queue != NULL);
    context->graphics_queue_family = queue_family_indices.graphics_family;

    vkGetDeviceQueue(context->device, queue_family_indices.present_family, 0, &context->present_queue);
    assert(context->present_queue != NULL);
    context->present_queue_family = queue_family_indices.present_family;

    return 0;
}

VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR *available_formats)
{
    TODO("Add better error handling and stuff");
    assert(available_formats != NULL);
    assert(GET_ARRAY_LENGTH(available_formats) > 0);

    for (size_t i = 0; i < GET_ARRAY_LENGTH(available_formats); i++)
    {
        VkSurfaceFormatKHR available_format = available_formats[i];
        if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *available_present_modes)
{
    TODO("Add better error handling and stuff");
    assert(available_present_modes != NULL);
    assert(GET_ARRAY_LENGTH(available_present_modes) > 0);

    for (size_t i = 0; i < GET_ARRAY_LENGTH(available_present_modes); i++)
    {
        VkPresentModeKHR available_present_mode = available_present_modes[i];
        // if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        TODO("Look into correct setting here");
        if (available_present_mode == VK_PRESENT_MODE_FIFO_KHR)
        {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

int32_t choose_swap_extent(LoggerInterface *logger, WindowInterface *window, VkSurfaceCapabilitiesKHR *capabilities,
                           VkExtent2D *out_extent)
{
    assert(logger != NULL);
    assert(window != NULL);
    assert(capabilities != NULL);
    assert(out_extent != NULL);
    int32_t ret;

    TODO("Check if this works correctly")
    if (capabilities->currentExtent.width != UINT32_MAX)
    {
        *out_extent = capabilities->currentExtent;
        return 0;
    }

    uint32_t width, height;
    RETURN_IF_ERROR(logger, ret, window_get_window_size(window, &width, &height),
                    "Failed to get window width and height: %d", ret);

    *out_extent = (VkExtent2D){
        .width = CLAMP(width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width),
        .height = CLAMP(height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height),
    };

    return 0;
}

void destroy_main_swapchain(RendererContext *context)
{
    assert(context != NULL);
    assert(context->swapchain != NULL);

    vkDestroySwapchainKHR(context->device, context->swapchain, NULL);
}

int32_t create_swapchain(RendererContext *context)
{
    assert(context != NULL);
    int32_t ret;
    VkResult result;

    SwapchainSupportDetails swapchain_support_details;
    SET_ARRAY_FIELD_CAPACITY(swapchain_support_details.surface_formats);
    SET_ARRAY_FIELD_CAPACITY(swapchain_support_details.present_modes);

    RETURN_IF_ERROR_RET_VALUE(context->deps.logger, ret, query_swapchain_support_details(context, context->physical_device, &swapchain_support_details),
                              false, "Failed to query swapchain support details: %d", ret);

    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swapchain_support_details.surface_formats);
    VkPresentModeKHR present_mode = choose_swap_present_mode(swapchain_support_details.present_modes);
    VkExtent2D surface_extent;
    RETURN_IF_ERROR(context->deps.logger, ret,
                    choose_swap_extent(context->deps.logger, context->deps.window, &swapchain_support_details.capabilities, &surface_extent),
                    "Failed to get surface extent: %d", ret);

    assert(surface_extent.width > 0);
    assert(surface_extent.height > 0);

    // Simply sticking to the minimum means that we may sometimes have to wait on the driver to complete internal operations before we can acquire another image to render to.
    // Therefore it is recommended to request at least one more image than the minimum.
    uint32_t swapchain_images_len = swapchain_support_details.capabilities.minImageCount + 1;
    uint32_t swapchain_images_max_len = swapchain_support_details.capabilities.maxImageCount;
    if (swapchain_images_max_len > 0 && swapchain_images_len > swapchain_images_max_len)
    {
        swapchain_images_len = swapchain_images_max_len;
    }

    TODO("Check if color attachment bit is necessary? vkguide.dev has no is weird")
    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = context->surface,
        .oldSwapchain = context->old_swapchain,

        .minImageCount = swapchain_images_len,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = surface_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        // swapchain_create_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    QueueFamilyIndices queue_family_indices = {0};
    RETURN_IF_ERROR_RET_VALUE(context->deps.logger, ret, find_queue_families(context, context->physical_device, &queue_family_indices),
                              false, "Failed to find queue family indices: %d", ret);

    uint32_t indices[2];
    if (queue_family_indices.graphics_family != queue_family_indices.present_family)
    {
        indices[0] = queue_family_indices.graphics_family;
        indices[1] = queue_family_indices.present_family;

        TODO("Currently this is slower as EXCLUSIVE is the more performant option, but also more complicated in this scenario, add this performant version")
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = indices;
    }
    else
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
    }

    swapchain_create_info.preTransform = swapchain_support_details.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkCreateSwapchainKHR(context->device, &swapchain_create_info, NULL, &context->swapchain),
                       -1, "Failed to create swapchain: %d", ret);

    if (context->old_swapchain == VK_NULL_HANDLE)
    {
        RETURN_IF_ERROR(context->deps.logger, ret,
                        RV_CALL_QUEUE_PUSH_1(context->deps.logger, &context->main_destroy_queue, destroy_main_swapchain, context),
                        "Failed to push swapchain destroy data to destroy queue: %d", ret);
    }
    else
    {
        vkDestroySwapchainKHR(context->device, context->old_swapchain, NULL);
    }

    SET_ARRAY_FIELD_CAPACITY(context->swapchain_images);
    assert(GET_ARRAY_CAPACITY(context->swapchain_images) >= swapchain_images_len);
    GET_ARRAY_LENGTH(context->swapchain_images) = (size_t)swapchain_images_len;
    vkGetSwapchainImagesKHR(context->device, context->swapchain, &swapchain_images_len, context->swapchain_images);

    context->swapchain_image_format = (RV_VkFormat)surface_format.format;
    context->swapchain_extent.width = surface_extent.width;
    context->swapchain_extent.height = surface_extent.height;
    context->old_swapchain = context->swapchain;

    return 0;
}

int32_t create_image_views(RendererContext *context)
{
    assert(context != NULL);
    SET_ARRAY_FIELD_CAPACITY(context->swapchain_image_views);
    assert(GET_ARRAY_CAPACITY(context->swapchain_image_views) == GET_ARRAY_CAPACITY(context->swapchain_images));
    GET_ARRAY_LENGTH(context->swapchain_image_views) = GET_ARRAY_LENGTH(context->swapchain_images);

    VkResult result;
    int32_t ret;

    VkImageViewCreateInfo image_view_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,

        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = (VkFormat)context->swapchain_image_format,

        .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,

        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };

    for (size_t i = 0; i < GET_ARRAY_LENGTH(context->swapchain_images); i++)
    {
        image_view_create_info.image = context->swapchain_images[i];

        VK_RETURN_IF_ERROR(context->deps.logger, result, vkCreateImageView(context->device, &image_view_create_info, NULL, &context->swapchain_image_views[i]),
                           -1, "Failed to create image view %d: %d", i, result);

        RETURN_IF_ERROR(context->deps.logger, ret,
                        RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->swapchain_destroy_queue, vkDestroyImageView, context->device, context->swapchain_image_views[i], NULL),
                        "Failed to push image view destroy data to destroy queue: %d", ret);
    }

    return 0;
}

int32_t renderer_vulkan_bootstrap(RendererContext *context)
{
    assert(context != NULL);

    if (context == NULL)
    {
        return -1;
    }

    int32_t ret;
    VK_TRY_INIT(context->deps.logger, ret, create_instance(context), &context->main_destroy_queue,
                "Failed to create instance: %d", ret);

#if IS_DEBUG
    VK_TRY_INIT(context->deps.logger, ret, setup_debug_messenger(context), &context->main_destroy_queue,
                "Failed to setup debug messenger: %d", ret);
#endif // #if IS_DEBUG

    VK_TRY_INIT(context->deps.logger, ret, create_surface(context), &context->main_destroy_queue,
                "Failed to create surface: %d", ret);
    VK_TRY_INIT(context->deps.logger, ret, pick_physical_device(context), &context->main_destroy_queue,
                "failed to pick physical device: %d", ret);
    VK_TRY_INIT(context->deps.logger, ret, create_logical_device(context), &context->main_destroy_queue,
                "Failed to create logical device: %d", ret);
    VK_TRY_INIT(context->deps.logger, ret, create_swapchain(context), &context->main_destroy_queue,
                "failed to create swapchain, %d", ret);
    VK_TRY_INIT(context->deps.logger, ret, create_image_views(context), &context->main_destroy_queue,
                "Failed to create image views: %d", ret);

    return 0;
}

int32_t renderer_vulkan_bootstrap_recreate_swapchain(RendererContext *context)
{
    TODO("Figure out if need to use VK_TRY_INIT or not");
    assert(context != NULL);
    int32_t ret;

    VK_TRY_INIT(context->deps.logger, ret, create_swapchain(context), &context->main_destroy_queue,
                "failed to recreate swapchain, %d", ret);
    VK_TRY_INIT(context->deps.logger, ret, create_image_views(context), &context->main_destroy_queue,
                "Failed to recreate image views: %d", ret);

    return 0;
}