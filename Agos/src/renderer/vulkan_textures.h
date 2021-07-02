#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
namespace Agos{
    class AgVulkanHandlerColorDepthRessourcesManager;
}
#include "Agos/src/renderer/vulkan_ressources.h"
#include "Agos/src/renderer/vulkan_command_pool.h"

#include AG_VULKAN_INCLUDE
#include <memory>
#include <vector>
#include <string>


namespace Agos
{

typedef class AG_API AgVulkanHandlerTextureManager
{
private:
    VkImage m_TextureImage;
    VkImageView m_TextureImageView;
    VkDeviceMemory m_TextureImageMemory;
    uint32_t m_MipLevels;
    VkSampler m_TextureSampler;

    VkDevice m_LogicalDeviceReference;
    bool m_Terminated = false;

public:
    AgVulkanHandlerTextureManager();
    AgVulkanHandlerTextureManager(const VkDevice& logical_device);
    ~AgVulkanHandlerTextureManager();

    AgResult create_texture_image(
        const std::string& texture_path,
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager,
        const std::shared_ptr<AgVulkanHandlerCommandPoolManager>& command_pool_manager);

    AgResult create_texture_image_view(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain);

    AgResult create_texture_sampler(
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device);

    AgResult terminate();

    VkImage& get_texture_image();
    VkImageView& get_texture_image_view();
    VkDeviceMemory& get_texture_image_memory();
    uint32_t& get_miplevels();
    VkSampler& get_texture_sampler();

private:
    void create_buffer(
        const VkPhysicalDevice& physical_device,
        const VkDevice& logical_device,
        const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager,
        const VkDeviceSize& size,
        const VkBufferUsageFlags& usage,
        const VkMemoryPropertyFlags& properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    void transition_image_layout(
        const VkDevice& logical_device,
        const VkQueue& graphics_queue,
        const VkCommandPool& command_pool,
        const VkImage& image,
        const VkFormat& format,
        const VkImageLayout& oldLayout,
        const VkImageLayout& newLayout,
        const uint32_t& mipLevels);

    void copy_buffer_to_image(
        const VkDevice& logical_device,
        const VkQueue& graphics_queue,
        const VkCommandPool& command_pool,
        const VkBuffer& buffer,
        const VkImage& image,
        const uint32_t& width,
        const uint32_t& height);

    void generate_mipmaps(
        const VkPhysicalDevice& physical_device,
        const VkDevice& logical_device,
        const VkQueue& graphics_queue,
        const VkCommandPool& command_pool,
        const VkImage& image,
        const VkFormat& imageFormat,
        const int32_t& texWidth,
        const int32_t& texHeight,
        const uint32_t& mipLevels);

    VkCommandBuffer begin_single_time_command(
        const VkDevice& logical_device,
        const VkCommandPool& commandPool);

    void end_single_time_command(
        const VkDevice& logical_device,
        const VkQueue& graphics_queue,
        const VkCommandPool& command_pool,
        const VkCommandBuffer& commandBuffer);

};
} // namespace Agos

