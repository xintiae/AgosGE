#include "Agos/src/renderer/vulkan_app/vulkan_helpers.h"

#include "Agos/src/renderer/vulkan_base/vulkan_base.h"
#include "Agos/src/logger/logger.h"

// ** images, image views, and buffers helper functions =============================================================================================
// * = = = = = = = = = = = = = = = = = = = = image and image view helper functions = = = = = = = = = = = = = = = = = = = =
Agos::VulkanHandler::VulkanHelpers::AgImage Agos::VulkanHandler::VulkanHelpers::create_image(
    const VkPhysicalDevice&         physical_device,
    const VkDevice&                 logical_device,
    const uint32_t&                 width,
    const uint32_t&                 height,
    const uint32_t&                 mipLevels,
    const VkSampleCountFlagBits&    numSamples,
    const VkFormat&                 format,
    const VkImageTiling&            tiling,
    const VkImageUsageFlags&        usage,
    const VkMemoryPropertyFlags&    properties,
    const VkAllocationCallbacks*    allocator /*= nullptr*/)
{
    Agos::VulkanHandler::VulkanHelpers::AgImage image;

    // VkImage creation
    VkImageCreateInfo imageInfo{};
    imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType         = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width      = width;
    imageInfo.extent.height     = height;
    imageInfo.extent.depth      = 1;
    imageInfo.mipLevels         = mipLevels;
    imageInfo.arrayLayers       = 1;
    imageInfo.format            = format;
    imageInfo.tiling            = tiling;
    imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage             = usage;
    imageInfo.samples           = numSamples;
    imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(logical_device, &imageInfo, allocator, &image.image) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - create_image] Failed to create image!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - create_image] Failed to create image!");
    }

    // VkImage mem's requirements for the VkImage to be bound to a VkDeviceMemory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logical_device, image.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize    = memRequirements.size;
    allocInfo.memoryTypeIndex   = Agos::VulkanHandler::VulkanBase::find_memory_type_index(physical_device, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logical_device, &allocInfo, allocator, &image.imageMemory) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL(           "[Vulkan/Agos::VulkanHandler::VulkanHelpers - create_image] Failed to allocate image memory!");
        throw std::runtime_error(   "[Vulkan/Agos::VulkanHandler::VulkanHelpers - create_image] Failed to allocate image memory!");
    }

    vkBindImageMemory(logical_device, image.image, image.imageMemory, 0);
    return image;
}

VkImageView Agos::VulkanHandler::VulkanHelpers::create_image_view(
    const VkDevice&                 logical_device,
    const VkImage&                  image,
    const VkFormat&                 format,
    const VkImageAspectFlags&       aspectFlags,
    const uint32_t&                 mipLevels,
    const VkAllocationCallbacks*    allocator /*= nullptr*/)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                              = image;
    viewInfo.viewType                           = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                             = format;
    viewInfo.subresourceRange.aspectMask        = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel      = 0;
    viewInfo.subresourceRange.levelCount        = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer    = 0;
    viewInfo.subresourceRange.layerCount        = 1;

    VkImageView imageView;
    if (vkCreateImageView(logical_device, &viewInfo, allocator, &imageView) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL(           "[Vulkan/Agos::VulkanHandler::VulkanHelpers - create_image_view] Failed to create texture image view!");
        throw std::runtime_error(   "[Vulkan/Agos::VulkanHandler::VulkanHelpers - create_image_view] Failed to create texture image view!");
    }

    return imageView;
}

void Agos::VulkanHandler::VulkanHelpers::transition_image_layout(
    const VkDevice&                 logical_device,
    const VkQueue&                  execution_queue,
    const VkCommandPool&            command_pool,
    const VkImage&                  image,
    const VkFormat&                 format,
    const VkImageLayout&            oldLayout,
    const VkImageLayout&            newLayout,
    const uint32_t&                 mipLevels)
{
    VkCommandBuffer commandBuffer = begin_single_time_command(logical_device, command_pool);

    VkImageMemoryBarrier barrier{};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

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
        AG_CORE_ERROR(              "[Vulkan/Agos::VulkanHandler::VulkanHelpers - transition_image_layout] Unsupported layout transition!");
        throw std::invalid_argument("[Vulkan/Agos::VulkanHandler::VulkanHelpers - transition_image_layout] Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    end_single_time_command(logical_device, command_pool, execution_queue, commandBuffer);
}

void Agos::VulkanHandler::VulkanHelpers::copy_buffer_to_image(
    const VkDevice&                 logical_device,
    const VkQueue&                  execution_queue,
    const VkCommandPool&            command_pool,
    const VkBuffer&                 buffer,
    const VkImage&                  image,
    const uint32_t&                 width,
    const uint32_t&                 height)
{
    VkCommandBuffer commandBuffer = begin_single_time_command(logical_device, command_pool);

    VkBufferImageCopy region{};
    region.bufferOffset                     = 0;
    region.bufferRowLength                  = 0;
    region.bufferImageHeight                = 0;
    region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel        = 0;
    region.imageSubresource.baseArrayLayer  = 0;
    region.imageSubresource.layerCount      = 1;
    region.imageOffset                      = { 0, 0, 0 };
    region.imageExtent                      = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    end_single_time_command(logical_device, command_pool, execution_queue, commandBuffer);
}


VkFormat Agos::VulkanHandler::VulkanHelpers::find_supported_format(
    const VkPhysicalDevice&         physical_device,
    const std::vector<VkFormat>&    candidates,
    VkImageTiling                   tiling,
    VkFormatFeatureFlags            features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    AG_CORE_CRITICAL(           "[Vulkan/Agos::VulkanHandler::VulkanHelpers - find_supported_format] Failed to find supported format!");
    throw std::runtime_error(   "[Vulkan/Agos::VulkanHandler::VulkanHelpers - find_supported_format] Failed to find supported format!");
}

VkFormat Agos::VulkanHandler::VulkanHelpers::find_depth_format(
    const VkPhysicalDevice&     physical_device)
{
    return find_supported_format(
        physical_device,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void Agos::VulkanHandler::VulkanHelpers::generate_mipmaps(
    const VkPhysicalDevice&         physical_device,
    const VkDevice&                 logical_device,
    const VkQueue&                  execution_queue,
    const VkCommandPool&            command_pool,
    const VkImage&                  image,
    const VkFormat&                 imageFormat,
    const int32_t&                  texWidth,
    const int32_t&                  texHeight,
    const uint32_t&                 mipLevels)
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physical_device, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        AG_CORE_ERROR(              "[Vulkan/Agos::VulkanHandler::VulkanHelpers - generate_mipmaps] Texture image format does not support linear blitting!");
        throw std::runtime_error(   "[Vulkan/Agos::VulkanHandler::VulkanHelpers - generate_mipmaps] Texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = begin_single_time_command(logical_device, command_pool);

    VkImageMemoryBarrier barrier{};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image                           = image;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;
    barrier.subresourceRange.levelCount     = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask   = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0]                  = {0, 0, 0};
        blit.srcOffsets[1]                  = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel        = i - 1;
        blit.srcSubresource.baseArrayLayer  = 0;
        blit.srcSubresource.layerCount      = 1;
        blit.dstOffsets[0]                  = {0, 0, 0};
        blit.dstOffsets[1]                  = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel        = i;
        blit.dstSubresource.baseArrayLayer  = 0;
        blit.dstSubresource.layerCount      = 1;

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
        command_pool,
        execution_queue,
        commandBuffer);
}

// * = = = = = = = = = = = = = = = = = = = = image and image viewhelper functions = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = buffer helper functions = = = = = = = = = = = = = = = = = = = =
Agos::VulkanHandler::VulkanHelpers::AgBuffer Agos::VulkanHandler::VulkanHelpers::create_buffer(
    const VkPhysicalDevice&         physical_device,
    const VkDevice&                 logical_device,
    const VkDeviceSize&             size,
    const VkBufferUsageFlags&       usage,
    const VkMemoryPropertyFlags&    properties,
    const VkAllocationCallbacks*    allocator /*= nullptr*/)
{
    VkBuffer        buffer;
    VkDeviceMemory  bufferMemory;

    // buffer creation process
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size         = size;
    bufferInfo.usage        = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logical_device, &bufferInfo, allocator, &buffer) != VK_SUCCESS)
    {
        AG_CORE_ERROR(              "[Vulkan/Agos::VulkanHandler::VulkanHelpers - create_buffer] Failed to create buffer!");
        throw std::runtime_error(   "[Vulkan/Agos::VulkanHandler::VulkanHelpers - create_buffer] Failed to create buffer!");
    }

    // buffer's mem requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logical_device, buffer, &memRequirements);

    // buffer's allocation
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize    = memRequirements.size;
    allocInfo.memoryTypeIndex   = Agos::VulkanHandler::VulkanBase::find_memory_type_index(physical_device, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logical_device, &allocInfo, allocator, &bufferMemory) != VK_SUCCESS)
    {
        AG_CORE_ERROR("[Vulkan/AgVulkanHandlerVIUBufferManager - create_buffer] Failed to allocate buffer memory!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerVIUBufferManager - create_buffer] Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(logical_device, buffer, bufferMemory, 0);
}

void Agos::VulkanHandler::VulkanHelpers::copy_buffer(
    const VkDevice&                 logical_device,
    const VkQueue&                  execution_queue,
    const VkCommandPool&            command_pool,
    const VkBuffer&                 src_buffer,
    const VkBuffer&                 dst_buffer,
    const VkDeviceSize&             size,
    const VkAllocationCallbacks*    allocator = nullptr)
{
    VkCommandBuffer commandBuffer = begin_single_time_command(logical_device, command_pool);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src_buffer, dst_buffer, 1, &copyRegion);

    end_single_time_command(logical_device, command_pool, execution_queue, commandBuffer);
}

void Agos::VulkanHandler::VulkanHelpers::destroy_buffer(
    const VkDevice&                                 logical_device,
    const Agos::VulkanHandler::VulkanHelpers::AgBuffer& buffer,
    const VkAllocationCallbacks*                    allocator /*= nullptr*/)
{
    vkFreeMemory(logical_device, buffer.bufferMemory, allocator);
    vkDestroyBuffer(logical_device, buffer.buffer, allocator);
}

VkCommandBuffer Agos::VulkanHandler::VulkanHelpers::begin_single_time_command(
    const VkDevice&         logical_device,
    const VkCommandPool&    command_pool
)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext                 = nullptr;
    allocInfo.commandPool           = command_pool;
    allocInfo.commandBufferCount    = 1;
    allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logical_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext             = nullptr;
    beginInfo.flags             = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo  = nullptr;  // we don't care if buffer is of primary level

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Agos::VulkanHandler::VulkanHelpers::end_single_time_command(
    const VkDevice&         logical_device,
    const VkCommandPool&    command_pool,
    const VkQueue&          execution_queue,
    const VkCommandBuffer&  command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &command_buffer;

    //  optional stuff
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount   = 0;
    submitInfo.pSignalSemaphores    = nullptr;
    submitInfo.pWaitSemaphores      = nullptr;
    // submitInfo.pWaitDstStageMask;

    vkQueueSubmit(execution_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(execution_queue);

    vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffer);
}

std::vector<VkCommandBuffer>&& Agos::VulkanHandler::VulkanHelpers::begin_single_time_commands(
    const VkDevice&         logical_device,
    const VkCommandPool&    command_pool,
    const size_t&           command_buffers_count)
{
    if (!command_buffers_count > 1 && !command_buffers_count < 101)
    {
        AG_CORE_WARN("[Vulkan/Agos::VulkanHandler::VulkanHelpers - begin_single_time_commands] command_buffers_count (which is "
                            + std::to_string(command_buffers_count) + ") is less than 1 or above 100!");
        return std::move(std::vector<VkCommandBuffer>());
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext                 = nullptr;
    allocInfo.commandPool           = command_pool;
    allocInfo.commandBufferCount    = command_buffers_count;
    allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    std::vector<VkCommandBuffer> commandBuffers(command_buffers_count);
    vkAllocateCommandBuffers(logical_device, &allocInfo, commandBuffers.data());


    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext             = nullptr;
    beginInfo.flags             = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo  = nullptr;  // we don't care if buffer is of primary level

    for (const VkCommandBuffer& commandBuffer : commandBuffers)
    {
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
    }

    return std::move(commandBuffers);
}
// * = = = = = = = = = = = = = = = = = = = = buffer helper functions = = = = = = = = = = = = = = = = = = = =

// ** images, image views, and buffers helper functions =============================================================================================
