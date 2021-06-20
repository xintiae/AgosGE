#include "Agos/src/debug_layers/vulkan_debug_layers.h"

#include "Agos/src/logger/logger.h"
#include <GLFW/glfw3.h>


std::vector<const char*> Agos::AgVulkanHandlerDebugLayersManager::m_ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};

Agos::AgVulkanHandlerDebugLayersManager::AgVulkanHandlerDebugLayersManager(const std::shared_ptr<Agos::AgVulkanHandlerInstance>& vulkanInstance)
    : m_VulkanInstanceReference(vulkanInstance)
{
}

Agos::AgVulkanHandlerDebugLayersManager::~AgVulkanHandlerDebugLayersManager()
{
    this->terminate();
}

Agos::AgResult Agos::AgVulkanHandlerDebugLayersManager::terminate()
{
    if (AG_ENABLE_DEBUG_VALIDATION_LAYER && !m_Terminated)
    {
        destroy_debug_utils_messenger_EXT(m_VulkanInstanceReference->get_instance(), m_DebugMessenger, nullptr);
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

VkDebugUtilsMessengerEXT& Agos::AgVulkanHandlerDebugLayersManager::get_debug_messenger()
{
    return m_DebugMessenger;
}

std::vector<const char*>& Agos::AgVulkanHandlerDebugLayersManager::get_validation_layers()
{
    return m_ValidationLayers;
}

// defines inside-private member std::shared_ptr<AgVulkanHandlerInstance>&
Agos::AgResult Agos::AgVulkanHandlerDebugLayersManager::vulkan_setup_debug_messenger()
{
    if (!AG_ENABLE_DEBUG_VALIDATION_LAYER)
        return Agos::AG_SUCCESS; // shall change to something like ?

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populate_debug_messenger_create_info(createInfo);
    if (create_debug_utils_messenger_EXT(m_VulkanInstanceReference->get_instance(), &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("failed to set up debug messenger!");
        return AG_FAILED_TO_SET_UP_VULKAN_DEBUG_MESSENGER;
    }

    return Agos::AG_SUCCESS;
}

void Agos::AgVulkanHandlerDebugLayersManager::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &debugMessengerCreateInfo)
{
    debugMessengerCreateInfo = {};
    debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMessengerCreateInfo.pfnUserCallback = ag_debug_callback;
}

VkResult Agos::AgVulkanHandlerDebugLayersManager::create_debug_utils_messenger_EXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT createMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (createMessenger != nullptr)
    {
        return createMessenger(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Agos::AgVulkanHandlerDebugLayersManager::destroy_debug_utils_messenger_EXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT destroyMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (destroyMessenger != nullptr)
    {
        destroyMessenger(instance, debugMessenger, pAllocator);
    }
}

bool Agos::AgVulkanHandlerDebugLayersManager::check_validation_layer_support()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : m_ValidationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char *> Agos::AgVulkanHandlerDebugLayersManager::get_required_extensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (AG_ENABLE_DEBUG_VALIDATION_LAYER)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL ag_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        AG_CORE_INFO(std::string("[Vulkan/debug callback] ") + std::string(pCallbackData->pMessage));
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        AG_CORE_INFO(std::string("[Vulkan/debug callback] ") + std::string(pCallbackData->pMessage));
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        AG_CORE_WARN(std::string("[Vulkan/debug callback] ") + std::string(pCallbackData->pMessage));
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        AG_CORE_ERROR(std::string("[Vulkan/debug callback] ") + std::string(pCallbackData->pMessage));
    }
    else
    {
        std::string callbackMessage = std::move(
            std::string("Received vulkan debug layer callback message, but severity (which is ") +
            std::to_string(messageSeverity) + std::string(") is unrecognized!\nMessage : \"") +
            std::string(pCallbackData->pMessage) + "\"");
        AG_CORE_WARN(callbackMessage);
    }

    return VK_FALSE;
}
