#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"
namespace Agos{
    class AgVulkanHandlerTextureManager;
}
#include "Agos/src/renderer/vulkan_textures.h"
namespace Agos{
    class AgVulkanHandlerBufferManager;
}
#include "Agos/src/renderer/vulkan_buffers.h"

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

    VkDevice& m_LogicalDeviceReference;
    bool m_Terminated = false;

public:
    AgVulkanHandlerColorDepthRessourcesManager();
    AgVulkanHandlerColorDepthRessourcesManager(VkDevice& logical_device);
    ~AgVulkanHandlerColorDepthRessourcesManager();

    AgResult create_color_ressources(
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain);
    AgResult create_depth_ressources(
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain);
    AgResult terminate();

    VkImage&         get_color_image();
    VkImageView&     get_color_image_view();
    VkFormat&        get_color_format();

    VkImage&         get_depth_image();
    VkImageView&     get_depth_image_view();
    VkFormat&        get_depth_format();

    friend class AgVulkanHandlerTextureManager;
    friend class AgVulkanHandlerVIUBufferManager;
protected:
    VkImage create_image(
       const VkPhysicalDevice& physical_device,
        const VkDevice& logical_device,
        const uint32_t& width,
        const uint32_t& height,
        const uint32_t& mipLevels,
        const VkSampleCountFlagBits& numSamples,
        const VkFormat& format,
        const VkImageTiling& tiling,
        const VkImageUsageFlags& usage,
        const VkMemoryPropertyFlags& properties,
        VkDeviceMemory& imageMemory);

    VkImageView create_image_view(
        const VkDevice& logical_device,
        const VkImage& image,
        const VkFormat& format,
        const VkImageAspectFlags& aspectFlags,
        const uint32_t& mipLevels);

    uint32_t find_memory_type(
        const VkPhysicalDevice& physical_device,
        const uint32_t& type_filter,
        const VkMemoryPropertyFlags& properties);

private:
    VkFormat find_depth_format(
        const VkPhysicalDevice& physical_device);

    VkFormat find_supported_format(
        const VkPhysicalDevice& physical_device,
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features
    );

} AgVulkanHandlerColorDepthRessourcesManager;

}   // namespace Agos
