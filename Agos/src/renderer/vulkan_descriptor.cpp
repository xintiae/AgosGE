#include "Agos/src/renderer/vulkan_descriptor.h"

#include "Agos/src/logger/logger.h"

Agos::AgVulkanHandlerDescriptorManager::AgVulkanHandlerDescriptorManager()
    : m_LogicalDeviceReference(AG_DEFAULT_LOGICAL_DEVICE_REFERENCE)
{
}

Agos::AgVulkanHandlerDescriptorManager::AgVulkanHandlerDescriptorManager(VkDevice& logical_device)
    : m_LogicalDeviceReference(logical_device)
{
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

Agos::AgResult Agos::AgVulkanHandlerDescriptorManager::create_descritpor_pool(
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain
)
{
    m_LogicalDeviceReference = logical_device->get_device();

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS);
    // poolSizes[0].descriptorCount = static_cast<uint32_t>(swapchain->get_swapchain_images().size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS);
    // poolSizes[1].descriptorCount = static_cast<uint32_t>(swapchain->get_swapchain_images().size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS);
    // poolInfo.maxSets = static_cast<uint32_t>(swapchain->get_swapchain_images().size());

    if (vkCreateDescriptorPool(logical_device->get_device(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerDescriptorManager - create_descriptor_pool] Failed to create descriptor pool!");
        return AG_FAILED_TO_CREATE_DESCRIPTOR_POOL;
    }
    AG_CORE_INFO("[Vulkan/AgVulkanHandlerDescriptorManager - create_descriptor_pool] Created descriptor pool!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerDescriptorManager::create_descriptor_sets(
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
    const std::shared_ptr<AgVulkanHandlerTextureManager>& texture_manager,
    const std::shared_ptr<AgVulkanHandlerVIUBufferManager>& buffer_manager,
    const uint32_t& model_index
)
{
    m_LogicalDeviceReference = logical_device->get_device();

    std::vector<VkDescriptorSetLayout> layouts(swapchain->get_swapchain_images().size(), m_DescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchain->get_swapchain_images().size());
    allocInfo.pSetLayouts = layouts.data();

    m_DescriptorsSets.resize(model_index + 1);
    m_DescriptorsSets[model_index].resize(swapchain->get_swapchain_images().size());
    if (vkAllocateDescriptorSets(logical_device->get_device(), &allocInfo, m_DescriptorsSets[model_index].data()) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerDescriptorManager - create_descriptor_sets] Failed to allocate descriptor sets!");
        return AG_FAILED_TO_ALLOCATE_DESCRIPTOR_SETS;
    }

    for (size_t i = 0; i < swapchain->get_swapchain_images().size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer_manager->get_uniform_buffers()[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Agos::VulkanGraphicsPipeline::UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture_manager->get_texture_image_view();
        imageInfo.sampler = texture_manager->get_texture_sampler();

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_DescriptorsSets[model_index][i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_DescriptorsSets[model_index][i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(logical_device->get_device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerDescriptorManager - create_descriptor_sets] Created descriptor sets!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerDescriptorManager::terminate_descriptor_set_layout()
{
    if (!m_DescriptorSetLayoutTerminated)
    {
        vkDestroyDescriptorSetLayout(m_LogicalDeviceReference, m_DescriptorSetLayout, nullptr);
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerDescriptorManager - terminate_descriptor_set_layout] Destroyed descriptor set layout!");
        m_DescriptorSetLayoutTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::AgVulkanHandlerDescriptorManager::terminate_descriptor_pool()
{
    if (!m_DescriptorPoolTerminated)
    {
        vkDestroyDescriptorPool(m_LogicalDeviceReference, m_DescriptorPool, nullptr);
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerDescriptorManager - terminate_descriptor_pool] Destroyed descriptor pool!");
        m_DescriptorPoolTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::AgVulkanHandlerDescriptorManager::terminate_descriptor_sets()
{
    if (!m_DescriptorsSetsTerminated)
    {
        // no need to destroy it anyway but, heh
        m_DescriptorsSetsTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::AgVulkanHandlerDescriptorManager::terminate()
{
    if (!m_Terminated)
    {
        terminate_descriptor_sets();
        terminate_descriptor_pool();
        terminate_descriptor_set_layout();
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerDescriptorManager - terminate] Terminated instance!");
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

VkDescriptorSetLayout& Agos::AgVulkanHandlerDescriptorManager::get_descriptor_set_layout()
{
    return m_DescriptorSetLayout;
}

std::vector<VkDescriptorSet>& Agos::AgVulkanHandlerDescriptorManager::get_descriptor_sets(const uint32_t& model_index)
{
    return m_DescriptorsSets[model_index];
}
