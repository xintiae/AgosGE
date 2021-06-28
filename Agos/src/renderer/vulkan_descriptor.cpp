#include "Agos/src/renderer/vulkan_descriptor.h"

#include "Agos/src/logger/logger.h"

Agos::AgVulkanHandlerDescriptorManager::AgVulkanHandlerDescriptorManager()
{
}

Agos::AgVulkanHandlerDescriptorManager::AgVulkanHandlerDescriptorManager(const VkDevice& logical_device)
{
    m_LogicalDeviceReference = logical_device;
}

Agos::AgVulkanHandlerDescriptorManager::~AgVulkanHandlerDescriptorManager()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerDescriptorManager::create_descriptor_set_layout(
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device)
{
    m_LogicalDeviceReference = logical_device->get_device();

    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.pImmutableSamplers = nullptr;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ubo_layout_binding, sampler_layout_binding};
    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logical_device->get_device(), &layout_info, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerDescriptorManager - create_descriptor_set_layout] Failed to create descriptor set layout!");
        return AG_FAILED_TO_CREATE_DESCRIPTOR_SET_LAYOUT;
    }

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerDescriptorManager - create_descriptor_set_layout] Created descriptor set layout!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerDescriptorManager::terminate()
{
    if (!m_Terminated)
    {
        vkDestroyDescriptorSetLayout(m_LogicalDeviceReference, m_DescriptorSetLayout, nullptr);
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
