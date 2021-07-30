#include "Agos/src/renderer/vulkan_command_pool.h"

#include "Agos/src/logger/logger.h"

extern VkDevice        AG_DEFAULT_LOGICAL_DEVICE_REFERENCE;


Agos::AgVulkanHandlerCommandPoolManager::AgVulkanHandlerCommandPoolManager()
    : m_LogicalDeviceReference(AG_DEFAULT_LOGICAL_DEVICE_REFERENCE)
{
}

Agos::AgVulkanHandlerCommandPoolManager::AgVulkanHandlerCommandPoolManager(VkDevice& logical_device)
    : m_LogicalDeviceReference(logical_device)
{
}

Agos::AgVulkanHandlerCommandPoolManager::~AgVulkanHandlerCommandPoolManager()
{
}

Agos::AgResult Agos::AgVulkanHandlerCommandPoolManager::create_command_pool(
    const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance,
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device)
{
    m_LogicalDeviceReference = logical_device->get_device();

    Agos::VulkanPhysicalDevice::AgQueueFamilyIndices queue_family_indices = physical_device->find_queue_families(physical_device->get_device(), glfw_instance);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queue_family_indices.graphics_family.value();

    if (vkCreateCommandPool(logical_device->get_device(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerCommandPoolManager - create_command_pool] Failed to create graphics command pool!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerCommandPoolManager - create_command_pool] Failed to create graphics command pool!");
    }
    AG_CORE_INFO("[Vulkan/AgVulkanHandlerCommandPoolManager - create_command_pool] Created graphics command pool!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerCommandPoolManager::terminate()
{
    if (!m_Terminated)
    {
        vkDestroyCommandPool(m_LogicalDeviceReference, m_CommandPool, nullptr);
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerCommandPoolManager - terminate] Destroyed command pool!");
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

VkCommandPool& Agos::AgVulkanHandlerCommandPoolManager::get_command_pool()
{
    return m_CommandPool;
}
