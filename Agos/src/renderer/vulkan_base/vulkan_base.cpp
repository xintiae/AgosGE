#include "Agos/src/renderer/vulkan_base/vulkan_base.h"

#include "Agos/src/logger/logger.h"
#include <set>

Agos::VulkanHandler::VulkanBase::VulkanBase(std::shared_ptr<GLFWHandler::GLFWInstance>& glfw_instance)
    : m_GLFWInstanceRef         (glfw_instance),
    m_Allocator                 (nullptr),
    m_FramebufferResizedFlag    (false),
    m_InstanceDestroyed         (false),
    m_DebugMessengerTerminated  (false),
    m_AllocatorTerminated       (false),
    m_PhysicalDeviceDestroyed   (false),
    m_LogicalDeviceDestroyed    (false),
    m_WindowSurfaceDestroyed    (false),
    m_VulkanBaseTerminated      (false)
{
}

Agos::VulkanHandler::VulkanBase::~VulkanBase()
{
    terminate_vulkan_base();
}


// ** init vulkan base ==============================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanBase::setup_vulkan_base()
{
    setup_allocation_callbacks();
    create_instance();
    setup_debug_messenger();
    setup_window_surface();
    pick_physical_device();
    create_logical_device();
    return AG_SUCCESS;
}


// ** destroy vulkan base ===========================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanBase::terminate_vulkan_base()
{
    if (!m_VulkanBaseTerminated)
    {
        destroy_logical_device();
        destroy_physical_device();
        terimnate_window_surface();
        terminate_debug_messenger();
        destroy_instance();
        terminate_allocation_callbacks();
        m_VulkanBaseTerminated = false;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}


// ** allocator managment ===========================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanBase::setup_allocation_callbacks()
{
    // ~ WIP
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanBase::terminate_allocation_callbacks()
{
    // ~ WIP
    return AG_SUCCESS;
}

// ** allocator managment ===========================================================================================================================


// ** instance managment ============================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanBase::create_instance()
{
    if (AG_ENABLE_DEBUG_VALIDATION_LAYER && !Agos::VulkanHandler::DebugLayersManager::check_validation_layer_support())
    {
        AG_CORE_ERROR("validation layers requested, but not available!");
        return AG_VALIDATION_LAYERS_NOT_AVAILABLE;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName    = "AgosGE";    // setup default name?
    appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName         = "AgosGE";
    appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion          = VK_API_VERSION_1_0;


    // extensions
    std::vector<const char *> extensions = Agos::VulkanHandler::DebugLayersManager::get_required_extensions();
    VkInstanceCreateInfo createInfo{};
    createInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo         = &appInfo;
    createInfo.enabledExtensionCount    = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames  = extensions.data();


    // debug layers
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (AG_ENABLE_DEBUG_VALIDATION_LAYER)
    {
        createInfo.enabledLayerCount    = static_cast<uint32_t>(Agos::VulkanHandler::DebugLayersManager::get_validation_layers().size());
        createInfo.ppEnabledLayerNames  = Agos::VulkanHandler::DebugLayersManager::get_validation_layers().data();

        Agos::VulkanHandler::DebugLayersManager::populate_debug_messenger_create_info(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    // final creation
    if (vkCreateInstance(&createInfo, m_Allocator, &m_Instance) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("failed to create vulkan instance!");
        return AG_FAILED_TO_CREATE_VULKAN_INSTANCE;
    }

    return AG_SUCCESS;    
}

Agos::AgResult Agos::VulkanHandler::VulkanBase::destroy_instance()
{
    if (!m_InstanceDestroyed)
    {
        vkDestroyInstance(m_Instance, m_Allocator);
        m_InstanceDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// ** instance managment ============================================================================================================================


// ** debug messenger managment =====================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanBase::setup_debug_messenger()
{
    return Agos::VulkanHandler::DebugLayersManager::vulkan_setup_debug_messenger(m_Instance, m_DebugMessenger, m_Allocator);
}

Agos::AgResult Agos::VulkanHandler::VulkanBase::terminate_debug_messenger()
{
    if (AG_ENABLE_DEBUG_VALIDATION_LAYER && !m_DebugMessengerTerminated)
    {
        Agos::VulkanHandler::DebugLayersManager::destroy_debug_utils_messenger_EXT(m_Instance, m_DebugMessenger, m_Allocator);
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerDebugLayersManager - terminate] Destroyed debug utils messenger!");
        m_DebugMessengerTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// ** debug messenger managment =====================================================================================================================


// ** window surface managment ======================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanBase::setup_window_surface()
{
    if (glfwCreateWindowSurface(m_Instance, m_GLFWInstanceRef->get_window(), m_Allocator, &m_WindowSurface) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan - GLFW/VulkanHandler::VulkanBase - setup_window_surface] Failed to setup window surface for VulkanBase entity!");
        return AG_FAILED_TO_CREATE_WINDOW_SURFACE;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::VulkanHandler::VulkanBase::terimnate_window_surface()
{
    if (!m_WindowSurfaceDestroyed)
    {
        vkDestroySurfaceKHR(m_Instance, m_WindowSurface, m_Allocator);
        m_WindowSurfaceDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// ** window surface managment ======================================================================================================================


// ** physical device managment =====================================================================================================================
const std::vector<const char*> Agos::VulkanHandler::VulkanBase::m_DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

Agos::AgResult Agos::VulkanHandler::VulkanBase::pick_physical_device()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanBase - pick_pysical_device] Failed to find vulkan compatible GPUs!");
        return AG_NO_VULKAN_COMPATIBLE_GPU_FOUND;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    for (const VkPhysicalDevice& device : devices)
    {
        if (is_device_suitable(device, m_WindowSurface))
        {
            m_PhysicalDevice = device;
            m_MsaaSamples = get_max_usable_sample_count(m_PhysicalDevice);
            break;
        }
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE)
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanBase - pick_pysical_device] Failed to find a suitable GPU!");
        return AG_FAILED_TO_FIND_SUITABLE_GPU;
    }
    AG_CORE_INFO("[Vulkan/VulkanHandler::VulkanBase - pick_physical_device] Found vulkan compatible GPU!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanBase::destroy_physical_device()
{
    return AG_INSTANCE_ALREADY_TERMINATED;
}

// * = = = = = = = = = = physical device's extensions & properties (queue families, their indices, etc...) = = = = = = = = = =
// suitable for graphics op (graphics and presentation queues, swapchain extension, MSAA)
bool Agos::VulkanHandler::VulkanBase::is_device_suitable(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface)
{
    // * does the specified physical device has ...

    // graphics and presentation queues?
    QueueFamiliesIndices indices = find_queue_families_indices(physical_device, surface);

    // swapchain extension?
    bool extensionsSupported = check_device_extensions_support(physical_device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapchainSupportDetails swapchainSupport = query_swapchain_support(physical_device, surface);
        swapChainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.present_modes.empty();
    }

    // MSAA feature - even a 1 bit one -?
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physical_device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

Agos::VulkanHandler::VulkanBase::QueueFamiliesIndices Agos::VulkanHandler::VulkanBase::find_queue_families_indices(
    const VkPhysicalDevice& physical_device,
    const VkSurfaceKHR& surface)
{
    Agos::VulkanHandler::VulkanBase::QueueFamiliesIndices indices;

    // querying queue family propreties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, queueFamilies.data());

    // here, we look for each family's indices (graphics, present, transfer)
    int i = 0;
    for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
        }
        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            indices.transfer_family = i;
        }
        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            indices.compute_family = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &presentSupport);

        if (presentSupport)
        {
            indices.present_family = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

bool Agos::VulkanHandler::VulkanBase::check_device_extensions_support(
    const VkPhysicalDevice& physical_device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

Agos::VulkanHandler::VulkanBase::SwapchainSupportDetails Agos::VulkanHandler::VulkanBase::query_swapchain_support(
    const VkPhysicalDevice& physical_device,
    const VkSurfaceKHR& surface)
{
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.present_modes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, details.present_modes.data());
    }

    return details;
}

VkSampleCountFlagBits Agos::VulkanHandler::VulkanBase::get_max_usable_sample_count(const VkPhysicalDevice& physical_device)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physical_device, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT)
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT)
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT)
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

uint32_t Agos::VulkanHandler::VulkanBase::find_memory_type_index(
    const VkPhysicalDevice& physical_device,
    const uint32_t& type_filter,
    const VkMemoryPropertyFlags& properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    AG_CORE_CRITICAL(           "[Vulkan/Agos::VulkanHandler::VulkanBase - find_memory_type] Failed to find suitable memory type!");
    throw std::runtime_error(   "[Vulkan/Agos::VulkanHandler::VulkanBase - find_memory_type] Failed to find suitable memory type!");
}

// * = = = = = = = = = = physical device's extensions & properties (queue families, their indices, etc...) = = = = = = = = = =
// ** physical device managment =====================================================================================================================


// ** logical device managment ======================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanBase::create_logical_device()
{
    // query each queue families' indicies (graphics, presentation, transfer, compute)
    Agos::VulkanHandler::VulkanBase::QueueFamiliesIndices indices = find_queue_families_indices(m_PhysicalDevice, m_WindowSurface);

    // setup each VkDeviceQueueCreateInfo per queue Families
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphics_family.value(),
        indices.present_family.value() ,
        indices.transfer_family.value(),
        indices.compute_family.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex    = queueFamily;
        queueCreateInfo.queueCount          = 1;
        queueCreateInfo.pQueuePriorities    = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // device's features (only MSAA enabled)
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // logical device create info struct
    VkDeviceCreateInfo createInfo{};
    createInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount     = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos        = queueCreateInfos.data();

    // features
    createInfo.pEnabledFeatures         = &deviceFeatures;

    // extensions
    createInfo.enabledExtensionCount    = static_cast<uint32_t>(m_DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames  = m_DeviceExtensions.data();

    // debugging and validation layers
    if (AG_ENABLE_DEBUG_VALIDATION_LAYER)
    {
        createInfo.enabledLayerCount    = static_cast<uint32_t>(Agos::VulkanHandler::DebugLayersManager::get_validation_layers().size() );
        createInfo.ppEnabledLayerNames  =                       Agos::VulkanHandler::DebugLayersManager::get_validation_layers().data()  ;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_PhysicalDevice, &createInfo, m_Allocator, &m_LogicalDevice) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanBase - create_logical_device] Failed to create logical device from GPU!");
        return AG_FAILED_TO_CREATE_LOGICAL_DEVICE_FROM_GPU;
    }

    // retrieve each VkQueue from the logical device we just created (graphics, presentation, transfer and compute queues)
    vkGetDeviceQueue(m_LogicalDevice, indices.graphics_family.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_LogicalDevice, indices.present_family.value() , 0, &m_PresentQueue );
    vkGetDeviceQueue(m_LogicalDevice, indices.transfer_family.value(), 0, &m_TransferQueue);
    vkGetDeviceQueue(m_LogicalDevice, indices.compute_family.value() , 0, &m_ComputeQueue );

    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanBase - create_logical_device] Created logical device!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanBase::destroy_logical_device()
{
    if (!m_LogicalDeviceDestroyed)
    {
        vkDestroyDevice(m_LogicalDevice, m_Allocator);
        m_LogicalDeviceDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// ** logical device managment ======================================================================================================================
