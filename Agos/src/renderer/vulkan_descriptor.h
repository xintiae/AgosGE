#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_instance.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"
#include "Agos/src/renderer/vulkan_textures.h"
#include "Agos/src/renderer/vulkan_buffers.h"

#include AG_VULKAN_INCLUDE
#include AG_GLM_INCLUDE
#include <memory>
#include <array>
#include <vector>


namespace Agos
{

namespace VulkanGraphicsPipeline
{
typedef struct AG_API UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
} UniformBufferObject;
}   // namespace VulkanGraphicsPipeline (within namespace Agos)

typedef class AG_API AgVulkanHandlerDescriptorManager
{
private:
    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkDescriptorPool m_DescriptorPool;
    std::vector<std::vector<VkDescriptorSet>> m_DescriptorsSets;

    VkDevice& m_LogicalDeviceReference;
    bool m_DescriptorSetLayoutTerminated    = false;
    bool m_DescriptorPoolTerminated         = false;
    bool m_DescriptorsSetsTerminated         = false;
    bool m_Terminated = false;

public:
    AgVulkanHandlerDescriptorManager();
    AgVulkanHandlerDescriptorManager(VkDevice& logical_device);
    ~AgVulkanHandlerDescriptorManager();

    AgResult create_descriptor_set_layout(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device);

    AgResult create_descritpor_pool(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain);

    AgResult create_descriptor_sets(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::shared_ptr<AgVulkanHandlerTextureManager>& texture_manager,
        const std::shared_ptr<AgVulkanHandlerVIUBufferManager>& buffer_manager,
        const uint32_t& model_index);

    AgResult terminate_descriptor_set_layout();
    AgResult terminate_descriptor_pool();
    AgResult terminate_descriptor_sets();
    AgResult terminate();

    VkDescriptorSetLayout& get_descriptor_set_layout();
    std::vector<VkDescriptorSet>& get_descriptor_sets(const uint32_t& model_index);

} AgVulkanHandlerDescriptorManager;
};
