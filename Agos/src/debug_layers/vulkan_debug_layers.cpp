#include "Agos/src/debug_layers/vulkan_debug_layers.h"

#include "Agos/src/logger/logger.h"
#include AG_GLFW_INCLUDE
#include <string>


const std::vector<const char*> Agos::VulkanHandler::DebugLayersManager::m_ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};

Agos::VulkanHandler::DebugLayersManager::DebugLayersManager()
{
}

Agos::VulkanHandler::DebugLayersManager::~DebugLayersManager()
{
}

Agos::AgResult Agos::VulkanHandler::DebugLayersManager::vulkan_setup_debug_messenger(
    const VkInstance& vulkan_instance,
    VkDebugUtilsMessengerEXT& debug_messenger,
    const VkAllocationCallbacks* allocation_callbacks)
{
    if (!AG_ENABLE_DEBUG_VALIDATION_LAYER)
        return Agos::AG_SUCCESS;    // shall change to something like ?

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populate_debug_messenger_create_info(createInfo);
    if (create_debug_utils_messenger_EXT(vulkan_instance, &createInfo, allocation_callbacks, &debug_messenger) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::DebugLayersManager - vulkan_setup_debug_messenger] Failed to set up debug messenger!");
        return AG_FAILED_TO_SET_UP_VULKAN_DEBUG_MESSENGER;
    }

    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::DebugLayersManager::vulkan_destroy_debug_messenger(
    const VkInstance& vulkan_instance,
    const VkDebugUtilsMessengerEXT& debug_messenger,
    const VkAllocationCallbacks* allocation_callbacks)
{
    if (AG_ENABLE_DEBUG_VALIDATION_LAYER)
    {
        destroy_debug_utils_messenger_EXT(vulkan_instance, debug_messenger, allocation_callbacks);
        AG_CORE_INFO("[Vulkan/VulkanHandler::DebugLayersManager - vulkan_destroy_debug_messenger] Destroyed debug utils messenger!");
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

const std::vector<const char*>& Agos::VulkanHandler::DebugLayersManager::get_validation_layers()
{
    return m_ValidationLayers;
}


void Agos::VulkanHandler::DebugLayersManager::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &debugMessengerCreateInfo)
{
    debugMessengerCreateInfo = {};
    debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMessengerCreateInfo.pfnUserCallback = Agos::VulkanHandler::DebugLayersManager::ag_debug_callback;
}

VkResult Agos::VulkanHandler::DebugLayersManager::create_debug_utils_messenger_EXT(
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

void Agos::VulkanHandler::DebugLayersManager::destroy_debug_utils_messenger_EXT(
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

bool Agos::VulkanHandler::DebugLayersManager::check_validation_layer_support()
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

std::vector<const char *> Agos::VulkanHandler::DebugLayersManager::get_required_extensions()
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

inline VKAPI_ATTR VkBool32 VKAPI_CALL AGOS_HELPER_ATTRIBUTE_UNUSED Agos::VulkanHandler::DebugLayersManager::ag_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT  *pCallbackData,
    void *                                      pUserData
)
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        AG_CORE_INFO(std::string("[Vulkan Debug Callback/VulkanHandler::DebugLayersManager - ag_debug_callback] ") + Agos::VulkanHandler::DebugLayersManager::ag_debug_callback_message_type(messageType) + " " + std::string(pCallbackData->pMessage));
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        AG_CORE_INFO(std::string("[Vulkan Debug Callback/VulkanHandler::DebugLayersManager - ag_debug_callback] ") + Agos::VulkanHandler::DebugLayersManager::ag_debug_callback_message_type(messageType) + " "  + std::string(pCallbackData->pMessage));
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        AG_CORE_WARN(std::string("[Vulkan Debug Callback/VulkanHandler::DebugLayersManager - ag_debug_callback] ") + Agos::VulkanHandler::DebugLayersManager::ag_debug_callback_message_type(messageType) + " "  + std::string(pCallbackData->pMessage));
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        AG_CORE_ERROR(std::string("[Vulkan Debug Callback/VulkanHandler::DebugLayersManager - ag_debug_callback] ") + Agos::VulkanHandler::DebugLayersManager::ag_debug_callback_message_type(messageType) + " " + std::string(pCallbackData->pMessage));
    }
    else
    {
        std::string callbackMessage = std::move(
            std::string("Received vulkan debug layer callback message, but message severity (which is ") +
            std::to_string(messageSeverity) + std::string(") is unrecognized!\nMessage type : ") + Agos::VulkanHandler::DebugLayersManager::ag_debug_callback_message_type(messageType) +
            std::string("\nMessage : \"") + std::string(pCallbackData->pMessage) + "\"");
        AG_CORE_WARN(callbackMessage);
    }
    AG_MARK_AS_USED(pUserData);
    return VK_FALSE;
}

inline std::string Agos::VulkanHandler::DebugLayersManager::ag_debug_callback_message_type(const VkDebugUtilsMessageTypeFlagsEXT& messageType)
{
    switch (messageType)
    {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        return std::move(std::string("[general]"));
        break;

    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        return std::move(std::string("[perormance]"));
        break;

    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        return std::move(std::string("[validation]"));
        break;

    default:
        return std::move(std::string("[message type unknown]"));
        break;
    }
    return std::move(std::string("[message type unknown]"));
}
