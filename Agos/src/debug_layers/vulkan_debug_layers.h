#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
namespace Agos{
    class AgVulkanHandlerInstance;
}
#include "Agos/src/renderer/vulkan_instance.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace Agos
{
typedef class AG_API AgVulkanHandlerDebugLayersManager
{
private:
    std::shared_ptr<AgVulkanHandlerInstance> m_VulkanInstanceReference;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    std::vector<const char*> m_ValidationLayers;

public:
    AgVulkanHandlerDebugLayersManager();
    AgVulkanHandlerDebugLayersManager(const std::shared_ptr<Agos::AgVulkanHandlerInstance>& vulkanInstance);
    ~AgVulkanHandlerDebugLayersManager();

    VkDebugUtilsMessengerEXT& get_debug_messenger();
    std::vector<const char*>& get_validation_layers();

    AgResult vulkan_setup_debug_messenger(const std::shared_ptr<AgVulkanHandlerInstance>& VulkanInstance);
    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo);

    bool check_validation_layer_support();
    std::vector<const char *> get_required_extensions();

private:
    VkResult create_debug_utils_messenger_EXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger
    );

    void destroy_debug_utils_messenger_EXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator
    );
} AgVulkanHandlerDebugLayersManager;

} // namespace Agos

static AG_API VKAPI_ATTR VkBool32 VKAPI_CALL ag_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData
);