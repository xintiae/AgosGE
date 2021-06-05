#include "Agos/src/renderer/vulkan_instance.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/debug_layers/vulkan_debug_layers.h"

VkInstance Agos::AgVulkanInstance;

Agos::AgResult Agos::ag_init_vulkan_instance()
{
    // vulkan logger?
    if (AG_ENABLE_DEBUG_VALIDATION_LAYER && !ag_check_validation_layer_support())
    {
        AG_CORE_ERROR("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "AgosGE";    // setup default name?
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "AgosGE";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char *> extensions = Agos::Ag_get_required_extensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (AG_ENABLE_DEBUG_VALIDATION_LAYER)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(Agos::AgValidationLayers.size());
        createInfo.ppEnabledLayerNames = Agos::AgValidationLayers.data();

        ag_populate_debug_messenger_create_info(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &Agos::AgVulkanInstance) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("failed to create vulkan instance!");
    }
    
    return Agos::AG_SUCCESS;
}
