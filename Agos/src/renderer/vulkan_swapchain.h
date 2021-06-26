#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/glfw_instance.h"
#include "Agos/src/renderer/vulkan_instance.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"

#include AG_VULKAN_INCLUDE
#include <memory>
#include <vector>

namespace Agos
{
typedef class AG_API AgVulkanHandlerSwapChain
{
private:
    VkSwapchainKHR m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;
    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent;

    // need it to destroy the swapchain
    // it was meant to be a reference, but getting compiler error when repporting it as a ref
    VkDevice m_LogicalDeviceReference;
    bool m_Terminated;

public:
    AgVulkanHandlerSwapChain();
    AgVulkanHandlerSwapChain(const VkDevice& logical_device);
    ~AgVulkanHandlerSwapChain();

    AgResult create_swap_chain(
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance);
    AgResult terminate();

private:
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D choose_swap_extent(
        const VkSurfaceCapabilitiesKHR& capabilities,
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance);
} AgVulkanHandlerSwapChain;

}   // namespace Agos

