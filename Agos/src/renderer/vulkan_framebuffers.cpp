#include "Agos/src/renderer/vulkan_framebuffers.h"

#include "Agos/src/logger/logger.h"

Agos::AgVulkanHandlerFramebuffers::AgVulkanHandlerFramebuffers()
{
    m_LogicalDeviceReference = VK_NULL_HANDLE;
}

Agos::AgVulkanHandlerFramebuffers::AgVulkanHandlerFramebuffers(const VkDevice& logical_device)
{
    m_LogicalDeviceReference = logical_device;
}

Agos::AgVulkanHandlerFramebuffers::~AgVulkanHandlerFramebuffers()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerFramebuffers::create_framebuffers(
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
    const std::shared_ptr<AgVulkanHandlerRenderPass>& render_pass,
    const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources)
{
    m_LogicalDeviceReference = logical_device->get_device();
    m_SwapChainFramebuffers.resize(swapchain->get_swapchain_image_views().size());

    for (size_t i = 0; i < swapchain->get_swapchain_image_views().size(); i++)
    {
        std::array<VkImageView, 3> attachments = {
            color_depth_ressources->get_color_image_view(),
            color_depth_ressources->get_depth_image_view(),
            swapchain->get_swapchain_image_views()[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = render_pass->get_render_pass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchain->get_swapchain_extent().width;
        framebufferInfo.height = swapchain->get_swapchain_extent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logical_device->get_device(), &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
        {
            AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerFramebuffers - create_framebuffers] Failed to create swap chain framebuffers!");
            return AG_FAILED_TO_CREATE_FRAMEBUFFERS;
        }
    }

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerFramebuffers - create_framebuffers] Created swap chain framebuffers!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerFramebuffers::terminate()
{
    if (!m_Terminated)
    {
        for (VkFramebuffer& frame_buffer : m_SwapChainFramebuffers)
        {
            vkDestroyFramebuffer(m_LogicalDeviceReference, frame_buffer, nullptr);
        }
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
