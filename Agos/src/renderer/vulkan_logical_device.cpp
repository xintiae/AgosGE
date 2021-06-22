#include "Agos/src/renderer/vulkan_logical_device.h"

#include <cstdint>
#include <set>

Agos::AgVulkanHandlerLogicalDevice::AgVulkanHandlerLogicalDevice()
{
}

Agos::AgVulkanHandlerLogicalDevice::~AgVulkanHandlerLogicalDevice()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerLogicalDevice::create_logical_device(
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance,
    const std::shared_ptr<AgVulkanHandlerDebugLayersManager>& debug_layers_manager)
{
    Agos::VulkanPhysicalDevice::AgQueueFamilyIndices indices = physical_device->find_queue_families(physical_device->get_device(), glfw_instance);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphics_family.value(), indices.present_family.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(physical_device->get_device_extensions().size());
    createInfo.ppEnabledExtensionNames = physical_device->get_device_extensions().data();

    if (AG_ENABLE_DEBUG_VALIDATION_LAYER)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(debug_layers_manager->get_validation_layers().size());
        createInfo.ppEnabledLayerNames = debug_layers_manager->get_validation_layers().data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device->get_device(), &createInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_LogicalDevice, indices.graphics_family.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_LogicalDevice, indices.present_family.value(), 0, &m_PresentQueue);

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerLogicalDevice::terminate()
{
    if (!m_Terminated)
    {
        vkDestroyDevice(m_LogicalDevice, nullptr);
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

VkDevice &Agos::AgVulkanHandlerLogicalDevice::get_device()
{
    return m_LogicalDevice;
}