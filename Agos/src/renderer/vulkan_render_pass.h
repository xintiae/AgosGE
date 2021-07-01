#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"

#include AG_VULKAN_INCLUDE
#include <memory>
#include <vector>


namespace Agos
{

typedef class AG_API AgVulkanHandlerRenderPass
{
private:
    VkRenderPass m_RenderPass;

    // like the swap chain, we need it to destroy the render pass
    // it was meant to be a reference, but getting compiler error when repporting it as a ref
    VkDevice m_LogicalDeviceReference;
    bool m_Terminated;

public:
    AgVulkanHandlerRenderPass();
    AgVulkanHandlerRenderPass(const VkDevice& logical_device);
    ~AgVulkanHandlerRenderPass();

    AgResult create_render_pass(
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain);
    AgResult terminate();
    VkRenderPass& get_render_pass();

private:
    VkFormat find_depth_format(
        const VkPhysicalDevice& physical_device
    );
    VkFormat find_supported_format(
        const std::vector<VkFormat>& candidates,
        const VkImageTiling& tiling,
        const VkFormatFeatureFlags& features,
        const VkPhysicalDevice& physical_device
    );
} AgVulkanHandlerRenderPass;

}   // namespace Agos
