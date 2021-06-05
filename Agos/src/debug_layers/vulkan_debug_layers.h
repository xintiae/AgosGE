#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace Agos
{
extern AG_API VkDebugUtilsMessengerEXT AgVulkanDebugMessenger;
extern AG_API VkInstance AgVulkanInstance;
extern AG_API const std::vector<const char *> AgValidationLayers;


AG_API AgResult ag_vulkan_setup_debug_messenger();


AG_API void ag_populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo);

AG_API VkResult ag_create_debug_utils_messenger_EXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger
);

AG_API void ag_destroy_debug_utils_messenger_EXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator
);

AG_API bool ag_check_validation_layer_support();
AG_API std::vector<const char *> Ag_get_required_extensions();

} // namespace Agos

static AG_API VKAPI_ATTR VkBool32 VKAPI_CALL ag_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData
);