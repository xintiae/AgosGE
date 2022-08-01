#include "Agos/src/renderer/vulkan_app/vulkan_entity.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/vulkan_app/vulkan_helpers.h"
#include AG_STB_INCLUDE

// ** Vulkan Vertex Entity ===========================================================================================
VkVertexInputBindingDescription Agos::VulkanHandler::VulkanEntity::Vertex::get_binding_description()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding      = 0;
    bindingDescription.stride       = sizeof(Vertex);
    bindingDescription.inputRate    = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 9> Agos::VulkanHandler::VulkanEntity::Vertex::get_attribute_description()
{
    std::array<VkVertexInputAttributeDescription, 9> attributeDescriptions{};

    // position
    attributeDescriptions[0].binding    = 0;
    attributeDescriptions[0].location   = 0;
    attributeDescriptions[0].format     = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset     = offsetof(Vertex, position);

    // color
    attributeDescriptions[1].binding    = 0;
    attributeDescriptions[1].location   = 1;
    attributeDescriptions[1].format     = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset     = offsetof(Vertex, color);

    // textCoord
    attributeDescriptions[2].binding    = 0;
    attributeDescriptions[2].location   = 2;
    attributeDescriptions[2].format     = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset     = offsetof(Vertex, textCoord);

    // ambtCoord
    attributeDescriptions[3].binding    = 0;
    attributeDescriptions[3].location   = 3;
    attributeDescriptions[3].format     = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset     = offsetof(Vertex, textCoord);

    // diffCoord
    attributeDescriptions[4].binding    = 0;
    attributeDescriptions[4].location   = 4;
    attributeDescriptions[4].format     = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[4].offset     = offsetof(Vertex, textCoord);

    // specCoord
    attributeDescriptions[5].binding    = 0;
    attributeDescriptions[5].location   = 5;
    attributeDescriptions[5].format     = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[5].offset     = offsetof(Vertex, textCoord);

    // nrmlCoord
    attributeDescriptions[6].binding    = 0;
    attributeDescriptions[6].location   = 6;
    attributeDescriptions[6].format     = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[6].offset     = offsetof(Vertex, textCoord);

    // alphCoord
    attributeDescriptions[7].binding    = 0;
    attributeDescriptions[7].location   = 7;
    attributeDescriptions[7].format     = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[7].offset     = offsetof(Vertex, textCoord);

    // normal
    attributeDescriptions[8].binding    = 0;
    attributeDescriptions[8].location   = 8;
    attributeDescriptions[8].format     = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[8].offset     = offsetof(Vertex, normal);

    return attributeDescriptions;
}
// ** Vulkan Vertex Entity ===========================================================================================

// ** Vulkan Texture Sampler =========================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanEntity::TextureSampler::create_sampler(
    const std::string&              image_path,
    const VkPhysicalDevice&         physical_device,
    const VkDevice&                 logical_device,
    const VkQueue&                  graphics_queue,
    const VkCommandPool&            allocation_command_pool,
    const VkAllocationCallbacks*    allocator)
{
    create_image(
        image_path,
        physical_device,
        logical_device,
        graphics_queue,
        allocation_command_pool,
        allocator
    );
    create_image_view(
        logical_device,
        allocator
    );
    create_image_sampler(
        physical_device,
        logical_device,
        allocator
    );

    m_Terminated = false;
    AG_CORE_INFO("[VulkanEntity/VulkanHandler::VulkanEntity::TextureSampler - create_image] Created image, image view and image sampler from specified image_path : " + image_path);
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanEntity::TextureSampler::terminate_sampler()
{
    if (!m_Terminated)
    {
        vkDestroySampler    (*m_SamplerDeviceReference, m_ImageSampler  , m_AllocatorReference);
        vkDestroyImageView  (*m_SamplerDeviceReference, m_ImageView     , m_AllocatorReference);
        vkDestroyImage      (*m_SamplerDeviceReference, m_Image         , m_AllocatorReference);
        vkFreeMemory        (*m_SamplerDeviceReference, m_ImageMemory   , m_AllocatorReference);

        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::VulkanHandler::VulkanEntity::TextureSampler::create_image(
    const std::string&              image_path,
    const VkPhysicalDevice&         physical_device,
    const VkDevice&                 logical_device,
    const VkQueue&                  graphics_queue,
    const VkCommandPool&            allocation_command_pool,
    const VkAllocationCallbacks*    allocator)
{
    m_SamplerDeviceReference    = &logical_device;
    m_AllocatorReference        = allocator;

    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(image_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels)
    {
        AG_CORE_ERROR("[VulkanEntity/VulkanHandler::VulkanEntity::TextureSampler - create_image] Failed to load image! (specified image_path : " + image_path + std::string(" )!"));
        return AG_FAILED;
    }

    VulkanHelpers::AgBuffer stagingBuffer = VulkanHelpers::create_buffer(
        physical_device,
        logical_device,
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        allocator
    );
    
    void* data;
    vkMapMemory(logical_device, stagingBuffer.bufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(logical_device, stagingBuffer.bufferMemory);

    stbi_image_free(pixels);

    VulkanHelpers::AgImage imageHelper = VulkanHelpers::create_image(
        physical_device,
        logical_device,
        texWidth,
        texHeight,
        m_MipLevels,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        allocator
    );

    m_Image       = imageHelper.image;
    m_ImageMemory = imageHelper.imageMemory;

    VulkanHelpers::transition_image_layout(
        logical_device,
        graphics_queue,
        allocation_command_pool,
        m_Image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        m_MipLevels
    );

    VulkanHelpers::copy_buffer_to_image(
        logical_device,
        graphics_queue,
        allocation_command_pool,
        stagingBuffer.buffer,
        m_Image,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight)
    );

    vkDestroyBuffer(logical_device, stagingBuffer.buffer, allocator);
    vkFreeMemory(logical_device, stagingBuffer.bufferMemory, allocator);

    VulkanHelpers::generate_mipmaps(
        physical_device,
        logical_device,
        graphics_queue,
        allocation_command_pool,
        m_Image,
        VK_FORMAT_R8G8B8A8_SRGB,
        texWidth,
        texHeight,
        m_MipLevels
    );

    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanEntity::TextureSampler::create_image_view(
    const VkDevice&                 logical_device,
    const VkAllocationCallbacks*    allocator
)
{
    m_ImageView = VulkanHelpers::create_image_view(
        logical_device,
        m_Image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_ASPECT_COLOR_BIT,
        m_MipLevels,
        allocator
    );

    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanEntity::TextureSampler::create_image_sampler(
    const VkPhysicalDevice&         physical_device,
    const VkDevice&                 logical_device,
    const VkAllocationCallbacks*    allocator
)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType                       = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter                   = VK_FILTER_LINEAR;
    samplerInfo.minFilter                   = VK_FILTER_LINEAR;
    samplerInfo.addressModeU                = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV                = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW                = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable            = VK_TRUE;
    samplerInfo.maxAnisotropy               = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor                 = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates     = VK_FALSE;
    samplerInfo.compareEnable               = VK_FALSE;
    samplerInfo.compareOp                   = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode                  = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod                      = 0.0f;
    samplerInfo.maxLod                      = static_cast<float>(m_MipLevels);
    samplerInfo.mipLodBias                  = 0.0f;

    if (vkCreateSampler(logical_device, &samplerInfo, allocator, &m_ImageSampler) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[VulkanEntity/VulkanHandler::VulkanEntity::TextureSampler - create_image] Failed to create texture sampler!");
        return AG_FAILED_TO_CREATE_TEXTURE_SAMPLER;
    }

    return AG_SUCCESS;
}

void Agos::VulkanHandler::VulkanEntity::TextureSampler::critical_assertion()
{
    if (!m_Terminated)
    {
        AG_CORE_ERROR("[VulkanEntity/VulkanHandler::VulkanEntity::TextureSampler - critical_assertion] Destructor called but sampler wasn't destroyed!");

        // AG_CORE_WARN("[VulkanEntity/VulkanHandler::VulkanEntity::TextureSampler - critical_assertion] Program behaviour is now undefined, bugs and crashes are likely to occure.");
    }
}
// ** Vulkan Texture Sampler =========================================================================================

