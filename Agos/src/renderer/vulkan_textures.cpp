#include "Agos/src/renderer/vulkan_textures.h"

#include AG_STB_INCLUDE
#include "Agos/src/logger/logger.h"

Agos::AgVulkanHandlerTextureImageManager::AgVulkanHandlerTextureImageManager()
{
}

Agos::AgVulkanHandlerTextureImageManager::AgVulkanHandlerTextureImageManager(const VkDevice& logical_device)
{
    m_LogicalDeviceReference = logical_device;
}

Agos::AgVulkanHandlerTextureImageManager::~AgVulkanHandlerTextureImageManager()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerTextureImageManager::create_texture_image(
    const std::string& texture_path,
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager,
    const std::shared_ptr<AgVulkanHandlerCommandPoolManager>& command_pool_manager
)
{
    m_LogicalDeviceReference = logical_device->get_device();

    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels)
    {
        AG_CORE_ERROR("[Vulkan/AgVulkanHandlerTextureImageManager - create_texture_image] Failed to load texture image!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerTextureImageManager - create_texture_image] Failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(
        physical_device->get_device(),
        logical_device->get_device(),
        color_depth_ressources_manager,
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *data;
    vkMapMemory(logical_device->get_device(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(logical_device->get_device(), stagingBufferMemory);

    stbi_image_free(pixels);

    m_TextureImage = color_depth_ressources_manager->create_image(
        physical_device->get_device(),
        logical_device->get_device(),
        texWidth,
        texHeight,
        m_MipLevels,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_TextureImageMemory);

    transition_image_layout(
        logical_device->get_device(),
        logical_device->get_graphics_queue(),
        command_pool_manager->get_command_pool(),
        m_TextureImage,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        m_MipLevels);

    copy_buffer_to_image(
        logical_device->get_device(),
        logical_device->get_graphics_queue(),
        command_pool_manager->get_command_pool(),
        stagingBuffer,
        m_TextureImage,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight));

    vkDestroyBuffer(logical_device->get_device(), stagingBuffer, nullptr);
    vkFreeMemory(logical_device->get_device(), stagingBufferMemory, nullptr);

    generate_mipmaps(
        physical_device->get_device(),
        logical_device->get_device(),
        logical_device->get_graphics_queue(),
        command_pool_manager->get_command_pool(),
        m_TextureImage,
        VK_FORMAT_R8G8B8A8_SRGB,
        texWidth,
        texHeight,
        m_MipLevels);
    
    AG_CORE_INFO("[Vulkan/AgVulkanHandlerTextureImageManager - create_texture_image] Created texture image from file : " + texture_path);
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerTextureImageManager::terminate()
{
    if (!m_Terminated)
    {
        vkDestroyImage(m_LogicalDeviceReference, m_TextureImage, nullptr);
        vkFreeMemory(m_LogicalDeviceReference, m_TextureImageMemory, nullptr);
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

void Agos::AgVulkanHandlerTextureImageManager::create_buffer(
    const VkPhysicalDevice& physical_device,
    const VkDevice& logical_device,
    const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager,
    const VkDeviceSize& size,
    const VkBufferUsageFlags& usage,
    const VkMemoryPropertyFlags& properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logical_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logical_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = color_depth_ressources_manager->find_memory_type(physical_device, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logical_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(logical_device, buffer, bufferMemory, 0);
}

void Agos::AgVulkanHandlerTextureImageManager::transition_image_layout(
    const VkDevice& logical_device,
    const VkQueue& graphics_queue,
    const VkCommandPool& command_pool,
    const VkImage& image,
    const VkFormat& format,
    const VkImageLayout& oldLayout,
    const VkImageLayout& newLayout,
    const uint32_t& mipLevels)
{
    VkCommandBuffer commandBuffer = begin_single_time_command(logical_device, command_pool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    end_single_time_command(
        logical_device,
        graphics_queue,
        command_pool,        
        commandBuffer);
}

void Agos::AgVulkanHandlerTextureImageManager::copy_buffer_to_image(
    const VkDevice& logical_device,
    const VkQueue& graphics_queue,
    const VkCommandPool& command_pool,
    const VkBuffer& buffer,
    const VkImage& image,
    const uint32_t& width,
    const uint32_t& height)
{
    VkCommandBuffer commandBuffer = begin_single_time_command(logical_device, command_pool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    end_single_time_command(
        logical_device,
        graphics_queue,
        command_pool,        
        commandBuffer);
}

void Agos::AgVulkanHandlerTextureImageManager::generate_mipmaps(
    const VkPhysicalDevice& physical_device,
    const VkDevice& logical_device,
    const VkQueue& graphics_queue,
    const VkCommandPool& command_pool,
    const VkImage& image,
    const VkFormat& imageFormat,
    const int32_t& texWidth,
    const int32_t& texHeight,
    const uint32_t& mipLevels)
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physical_device, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = begin_single_time_command(logical_device, command_pool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    end_single_time_command(
        logical_device,
        graphics_queue,
        command_pool,
        commandBuffer);
}


VkCommandBuffer Agos::AgVulkanHandlerTextureImageManager::begin_single_time_command(
    const VkDevice& logical_device,
    const VkCommandPool& commandPool
)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logical_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Agos::AgVulkanHandlerTextureImageManager::end_single_time_command(
    const VkDevice& logical_device,
    const VkQueue& graphics_queue,
    const VkCommandPool& command_pool,
    const VkCommandBuffer& commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(logical_device, command_pool, 1, &commandBuffer);
}
