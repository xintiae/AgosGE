#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_instance.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"

#include AG_VULKAN_INCLUDE
#include AG_GLM_INCLUDE
#include <memory>
#include <array>
#include <vector>


namespace Agos
{

namespace VulkanDescriptors
{
typedef struct AG_API UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
} UniformBufferObject;
}   // namespace VulkanDescriptors (within namespace Agos)

typedef class AG_API AgVulkanHandlerDescriptorManager
{
private:
    VkDescriptorSetLayout m_DescriptorSetLayout;

    // do I need to repeat myself?... (see Agos/src/renderer/vulkan_swapchain.h)
    VkDevice m_LogicalDeviceReference;
    bool m_Terminated = false;

public:
    AgVulkanHandlerDescriptorManager();
    AgVulkanHandlerDescriptorManager(const VkDevice& logical_device);
    ~AgVulkanHandlerDescriptorManager();

    AgResult create_descriptor_set_layout(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device
    );
    AgResult terminate();
    VkDescriptorSetLayout& get_descriptor_set_layout();

} AgVulkanHandlerDescriptorManager;
};
