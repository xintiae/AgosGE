#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/glfw_instance.h"
#include "Agos/src/renderer/vulkan_instance.h"
namespace Agos{
    class AgVulkanHandlerPhysicalDevice;
}
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_render_pass.h"
namespace Agos{
    class AgVulkanHandlerColorDepthRessourcesManager;
}
#include "Agos/src/renderer/vulkan_ressources.h"

#include AG_VULKAN_INCLUDE
#include <memory>
#include <vector>

namespace Agos
{
class AG_API AgVulkanHandlerSwapChain
{
private:
    VkSwapchainKHR m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<VkImageView> m_SwapChainImageViews;
    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent;
    std::vector<VkFramebuffer> m_SwapChainFramebuffers;

    VkDevice& m_LogicalDeviceReference;
    bool m_FramebuffersTerminated   = false;
    bool m_ImageViewsTerminated     = false;
    bool m_SwapChainTerminated      = false;
    bool m_Terminated   = false;

public:
    AgVulkanHandlerSwapChain();
    AgVulkanHandlerSwapChain(VkDevice& logical_device);
    ~AgVulkanHandlerSwapChain();

    AgResult create_swap_chain(
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance);
    AgResult create_image_views(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device
    );
    AgResult create_framebuffers(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerRenderPass>& render_pass,
        const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources);

    AgResult terminate_framebuffers                 (const bool& mark_as_terminated = true);
    AgResult terminate_swapchain_images_nd_views    (const bool& mark_as_terminated = true);
    AgResult terminate_swapchain                    (const bool& mark_as_terminated = true);
    AgResult terminate                              (const bool& mark_as_terminated = true);

    VkSwapchainKHR&             get_swapchain();
    std::vector<VkImage>&       get_swapchain_images();
    std::vector<VkImageView>&   get_swapchain_image_views();
    VkFormat&                   get_swapchain_image_format();
    VkExtent2D&                 get_swapchain_extent();
    std::vector<VkFramebuffer>& get_swapchain_framebuffers();


    friend class AgVulkanHandlerTextureManager;

private:
    VkSurfaceFormatKHR  choose_swap_surface_format  (const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR    choose_swap_present_mode    (const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D          choose_swap_extent          (
        const VkSurfaceCapabilitiesKHR& capabilities,
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance);

protected:
    VkImageView create_image_view(
        const VkImage& image,
        const VkFormat& format,
        const VkImageAspectFlags& aspectFlags,
        const uint32_t& mipLevels,
        const VkDevice& device
    );
};  // class AgVulkanHandlerSwapChain

}   // namespace Agos

