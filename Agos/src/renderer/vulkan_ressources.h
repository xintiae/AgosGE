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

typedef class AG_API AgVulkanHandlerColorDepthRessourcesManager
{
private:
    VkImage         m_ColorImage;
    VkImageView     m_ColorImageView;
    VkDeviceMemory  m_ColorImageMemory;
    VkFormat        m_ColorFormat;

    VkImage         m_DepthImage;
    VkImageView     m_DepthImageView;
    VkDeviceMemory  m_DepthImageMemory;
    VkFormat        m_DepthFormat;

    // like the swap chain, we need it to destroy the render pass
    VkDevice m_LogicalDeviceReference;
    bool m_Terminated;

public:
    AgVulkanHandlerColorDepthRessourcesManager();
    AgVulkanHandlerColorDepthRessourcesManager(const VkDevice& logical_device);
    ~AgVulkanHandlerColorDepthRessourcesManager();

    AgResult create_color_ressources();
    AgResult create_depth_ressources();
    AgResult terminate();

private:
    void create_image(
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels,
        VkSampleCountFlagBits numSamples,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory);

    VkImageView create_image_view(
        VkImage image,
        VkFormat format,
        VkImageAspectFlags aspectFlags,
        uint32_t mipLevels);
} AgVulkanHandlerColorDepthRessourcesManager;

}   // namespace Agos
