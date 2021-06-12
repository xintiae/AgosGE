#include "Agos/src/renderer/vulkan_instance.h"

#include "Agos/src/logger/logger.h"


Agos::AgVulkanHandlerInstance::AgVulkanHandlerInstance()
{
}

Agos::AgVulkanHandlerInstance::~AgVulkanHandlerInstance()
{
    this->destroy();
}

VkInstance& Agos::AgVulkanHandlerInstance::get_instance()
{
    return m_Instance;
}

Agos::AgResult Agos::AgVulkanHandlerInstance::init(const std::shared_ptr<AgVulkanHandlerDebugLayersManager>& DebugLayersManager)
{
    if (AG_ENABLE_DEBUG_VALIDATION_LAYER && !DebugLayersManager->check_validation_layer_support())
    {
        AG_CORE_ERROR("validation layers requested, but not available!");
        return AG_FAILED_TO_CREATE_VULKAN_INSTANCE;
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

    std::vector<const char *> extensions = DebugLayersManager->get_required_extensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (AG_ENABLE_DEBUG_VALIDATION_LAYER)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(DebugLayersManager->get_validation_layers().size());
        createInfo.ppEnabledLayerNames = DebugLayersManager->get_validation_layers().data();

        DebugLayersManager->populate_debug_messenger_create_info(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("failed to create vulkan instance!");
        return AG_FAILED_TO_CREATE_VULKAN_INSTANCE;
    }
    
    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerInstance::destroy()
{
    if (!m_Destroyed)
    {
        vkDestroyInstance(m_Instance, nullptr);
        m_Destroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
