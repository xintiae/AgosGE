#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include AG_VULKAN_INCLUDE
#include <vector>
#include <string>

namespace Agos
{

namespace VulkanHandler
{

class AG_API DebugLayersManager
{
private:
    static const std::vector<const char*> m_ValidationLayers;

public:
    DebugLayersManager();
    ~DebugLayersManager();

    // "create" debug messenger
    static AgResult vulkan_setup_debug_messenger(
        const VkInstance& vulkan_instance,
        VkDebugUtilsMessengerEXT& debug_messenger,
        const VkAllocationCallbacks* allocation_callbacks);

    // "destroy" debug messenger
    static AgResult vulkan_destroy_debug_messenger(
        const VkInstance& vulkan_instance,
        const VkDebugUtilsMessengerEXT& debug_messenger,
        const VkAllocationCallbacks* allocation_callbacks);

    static bool                                 check_validation_layer_support();
    static std::vector<const char *>            get_required_extensions();
    static const std::vector<const char*>&      get_validation_layers();


    // "setup and populate" debug messenger
    static void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo);

    static VkResult create_debug_utils_messenger_EXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger
    );

    static void destroy_debug_utils_messenger_EXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator
    );

    // actual debug messenger's callbacks
    inline static VKAPI_ATTR VkBool32 VKAPI_CALL AG_API AGOS_HELPER_ATTRIBUTE_UNUSED ag_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    );
    inline static AG_API AGOS_HELPER_ATTRIBUTE_UNUSED std::string ag_debug_callback_message_type(const VkDebugUtilsMessageTypeFlagsEXT& messageType);

};  // class DebugLayersManager

}   // namespace VulkanHandler (within namespace Agos)

} // namespace Agos
