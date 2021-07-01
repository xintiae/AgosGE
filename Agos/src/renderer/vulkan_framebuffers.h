#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"
#include "Agos/src/renderer/vulkan_render_pass.h"
#include "Agos/src/renderer/vulkan_ressources.h"

namespace Agos
{

typedef class AG_API AgVulkanHandlerFramebuffers
{
private:
    std::vector<VkFramebuffer> m_SwapChainFramebuffers;


    VkDevice m_LogicalDeviceReference;
    bool m_Terminated;

public:
    AgVulkanHandlerFramebuffers();
    AgVulkanHandlerFramebuffers(const VkDevice& logical_device);
    ~AgVulkanHandlerFramebuffers();

    AgResult create_framebuffers(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::shared_ptr<AgVulkanHandlerRenderPass>& render_pass,
        const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources);
    AgResult terminate();

} AgVulkanHandlerFramebuffers;

}   // namespace Agos
