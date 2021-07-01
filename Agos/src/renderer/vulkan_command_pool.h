#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/renderer/glfw_instance.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"


namespace Agos
{

typedef class AG_API AgVulkanHandlerCommandPoolManager
{
private:
    VkCommandPool m_CommandPool;

    VkDevice m_LogicalDeviceReference;
    bool m_Terminated;

public:
    AgVulkanHandlerCommandPoolManager();
    AgVulkanHandlerCommandPoolManager(const VkDevice& logical_device);
    ~AgVulkanHandlerCommandPoolManager();

    AgResult create_command_pool(
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance,
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain);
    AgResult terminate();

    VkCommandPool& get_command_pool();
};

}   // namespace Agos

