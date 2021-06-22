#include "Agos/src/renderer/vulkan_physical_device.h"

#include "Agos/src/logger/logger.h"
#include <set>

const std::vector<const char *> Agos::AgVulkanHandlerPhysicalDevice::m_DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

Agos::AgVulkanHandlerPhysicalDevice::AgVulkanHandlerPhysicalDevice()
{
}

Agos::AgVulkanHandlerPhysicalDevice::~AgVulkanHandlerPhysicalDevice()
{
}

Agos::AgResult Agos::AgVulkanHandlerPhysicalDevice::pick_physical_device(
    const std::shared_ptr<AgVulkanHandlerInstance> &vulkan_intance,
    const std::shared_ptr<AgGLFWHandlerInstance> &glfw_instance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkan_intance->get_instance(), &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerPhysicalDevice - pick_pysical_device] Failed to find vulkan compatible GPUs!");
        return AG_NO_VULKAN_COMPATIBLE_GPU_FOUND;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkan_intance->get_instance(), &deviceCount, devices.data());

    for (const auto &device : devices)
    {
        if (is_device_suitable(device, glfw_instance))
        {
            m_PhysicalDevice = device;
            m_MsaaSamples = get_max_usable_sample_count();
            break;
        }
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerPhysicalDevice - pick_pysical_device] Failed to find a suitable GPU!");
        return AG_FAILED_TO_FIND_SUITABLE_GPU;
    }
    AG_CORE_INFO("[Vulkan/AgVulkanHandlerPhysicalDevice] Found vulkan compatible GPU!");
    return AG_SUCCESS;
}

const std::vector<const char*>& Agos::AgVulkanHandlerPhysicalDevice::get_device_extensions()
{
    return m_DeviceExtensions;
}

VkPhysicalDevice& Agos::AgVulkanHandlerPhysicalDevice::get_device()
{
    return m_PhysicalDevice;
}

bool Agos::AgVulkanHandlerPhysicalDevice::is_device_suitable(
    const VkPhysicalDevice &physical_device,
    const std::shared_ptr<AgGLFWHandlerInstance> &glfw_instance)
{
    VulkanPhysicalDevice::AgQueueFamilyIndices indices = find_queue_families(physical_device, glfw_instance);

    bool extensionsSupported = check_device_extensions_support(physical_device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        VulkanPhysicalDevice::AgSwapChainSupportDetails swapChainSupport = query_swapchain_support(physical_device, glfw_instance);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physical_device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Agos::AgVulkanHandlerPhysicalDevice::check_device_extensions_support(const VkPhysicalDevice &physical_device)
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

Agos::VulkanPhysicalDevice::AgQueueFamilyIndices Agos::AgVulkanHandlerPhysicalDevice::find_queue_families(
    const VkPhysicalDevice &physical_device,
    const std::shared_ptr<AgGLFWHandlerInstance> &glfw_instance)
{
    VulkanPhysicalDevice::AgQueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, glfw_instance->get_surface(), &presentSupport);

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

Agos::VulkanPhysicalDevice::AgSwapChainSupportDetails Agos::AgVulkanHandlerPhysicalDevice::query_swapchain_support(
    const VkPhysicalDevice &physical_device,
    const std::shared_ptr<AgGLFWHandlerInstance> &glfw_instance)
{
    VulkanPhysicalDevice::AgSwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, glfw_instance->get_surface(), &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, glfw_instance->get_surface(), &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, glfw_instance->get_surface(), &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, glfw_instance->get_surface(), &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, glfw_instance->get_surface(), &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSampleCountFlagBits Agos::AgVulkanHandlerPhysicalDevice::get_max_usable_sample_count()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

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
