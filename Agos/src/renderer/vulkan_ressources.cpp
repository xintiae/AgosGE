#include "Agos/src/renderer/vulkan_ressources.h"


Agos::AgVulkanHandlerColorDepthRessourcesManager::AgVulkanHandlerColorDepthRessourcesManager()
{
}

Agos::AgVulkanHandlerColorDepthRessourcesManager::AgVulkanHandlerColorDepthRessourcesManager(const VkDevice& logical_device)
{
    m_LogicalDeviceReference = logical_device;
}

Agos::AgVulkanHandlerColorDepthRessourcesManager::~AgVulkanHandlerColorDepthRessourcesManager()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerColorDepthRessourcesManager::create_color_ressources(
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain
)
{
    m_LogicalDeviceReference = logical_device->get_device();
    VkFormat colorFormat = swapchain->get_swapchain_image_format();

    create_image(
        swapchain->get_swapchain_extent().width,
        swapchain->get_swapchain_extent().height,
        1,
        physical_device->get_msaa_samples(),
        colorFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_ColorImage,
        m_ColorImageMemory);
    
    m_ColorImageView = create_image_view(m_ColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerColorDepthRessourcesManager::create_depth_ressources(
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain
)
{
    VkFormat depthFormat = findDepthFormat();

    create_image(
        swapchain->get_swapchain_extent().width,
        swapchain->get_swapchain_extent().height,
        1,
        physical_device->get_msaa_samples(),
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_DepthImage, 
        m_DepthImageMemory);

    m_DepthImageView = createImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerColorDepthRessourcesManager::terminate()
{
    if (!m_Terminated)
    {

        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

void Agos::AgVulkanHandlerColorDepthRessourcesManager::create_image(
    uint32_t width,
    uint32_t height,
    uint32_t mipLevels,
    VkSampleCountFlagBits numSamples,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView createImageView(
    VkImage image, 
    VkFormat format,
    VkImageAspectFlags aspectFlags,
    uint32_t mipLevels)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}
