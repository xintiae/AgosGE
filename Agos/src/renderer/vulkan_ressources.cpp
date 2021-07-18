#include "Agos/src/renderer/vulkan_ressources.h"

#include "Agos/src/logger/logger.h"

extern VkDevice        AG_DEFAULT_LOGICAL_DEVICE_REFERENCE;

Agos::AgVulkanHandlerColorDepthRessourcesManager::AgVulkanHandlerColorDepthRessourcesManager()
    : m_LogicalDeviceReference(AG_DEFAULT_LOGICAL_DEVICE_REFERENCE)
{
}

Agos::AgVulkanHandlerColorDepthRessourcesManager::AgVulkanHandlerColorDepthRessourcesManager(VkDevice& logical_device)
    : m_LogicalDeviceReference(logical_device)
{
}

Agos::AgVulkanHandlerColorDepthRessourcesManager::~AgVulkanHandlerColorDepthRessourcesManager()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerColorDepthRessourcesManager::create_color_ressources(
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain)
{
    m_LogicalDeviceReference = logical_device->get_device();
    VkFormat colorFormat = swapchain->get_swapchain_image_format();

    m_ColorImage = create_image(
        physical_device->get_device(),
        logical_device->get_device(),
        swapchain->get_swapchain_extent().width,
        swapchain->get_swapchain_extent().height,
        1,
        physical_device->get_msaa_samples(),
        colorFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_ColorImageMemory);
    
    m_ColorImageView = create_image_view(logical_device->get_device(), m_ColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - create_color_ressources] Successfully created color ressources for msaa!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerColorDepthRessourcesManager::create_depth_ressources(
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain)
{
    VkFormat depthFormat = find_depth_format(physical_device->get_device());

    m_DepthImage = create_image(
        physical_device->get_device(),
        logical_device->get_device(),
        swapchain->get_swapchain_extent().width,
        swapchain->get_swapchain_extent().height,
        1,
        physical_device->get_msaa_samples(),
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_DepthImageMemory);

    m_DepthImageView = create_image_view(logical_device->get_device(), m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - create_depth_ressources] Successfully created depth ressources for msaa!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerColorDepthRessourcesManager::terminate(const bool& mark_as_terminated)
{
    if (!m_Terminated)
    {
        vkDestroyImageView(m_LogicalDeviceReference, m_DepthImageView, nullptr);
        vkDestroyImage(m_LogicalDeviceReference, m_DepthImage, nullptr);
        vkFreeMemory(m_LogicalDeviceReference, m_DepthImageMemory, nullptr);

        vkDestroyImageView(m_LogicalDeviceReference, m_ColorImageView, nullptr);
        vkDestroyImage(m_LogicalDeviceReference, m_ColorImage, nullptr);
        vkFreeMemory(m_LogicalDeviceReference, m_ColorImageMemory, nullptr);

        AG_CORE_INFO("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - terminate]" + std::string(
            " Destroyed depth image, depth image view, color image, color image view ; Freed depth image memory, color image memory"));

        m_Terminated = mark_as_terminated;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

VkImage& Agos::AgVulkanHandlerColorDepthRessourcesManager::get_color_image()
{
    return m_ColorImage;
}

VkImageView& Agos::AgVulkanHandlerColorDepthRessourcesManager::get_color_image_view()
{
    return m_ColorImageView;
}

VkFormat& Agos::AgVulkanHandlerColorDepthRessourcesManager::get_color_format()
{
    return m_ColorFormat;
}

VkImage& Agos::AgVulkanHandlerColorDepthRessourcesManager::get_depth_image()
{
    return m_DepthImage;
}

VkImageView& Agos::AgVulkanHandlerColorDepthRessourcesManager::get_depth_image_view()
{
    return m_DepthImageView;
}

VkFormat& Agos::AgVulkanHandlerColorDepthRessourcesManager::get_depth_format()
{
    return m_DepthFormat;
}

VkImage Agos::AgVulkanHandlerColorDepthRessourcesManager::create_image(
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
    VkDeviceMemory& imageMemory)
{
    VkImage image;

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

    if (vkCreateImage(logical_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - create_image] Failed to create image!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - create_image] Failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logical_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(physical_device, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logical_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - create_image] Failed to allocate image memory!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - create_image] Failed to allocate image memory!");
    }

    vkBindImageMemory(logical_device, image, imageMemory, 0);
    return image;
}

VkImageView Agos::AgVulkanHandlerColorDepthRessourcesManager::create_image_view(
    const VkDevice& logical_device,
    const VkImage& image, 
    const VkFormat& format,
    const VkImageAspectFlags& aspectFlags,
    const uint32_t& mipLevels)
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
    if (vkCreateImageView(logical_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - create_image_view] Failed to create texture image view!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - create_image_view] Failed to create texture image view!");
    }

    return imageView;
}

uint32_t Agos::AgVulkanHandlerColorDepthRessourcesManager::find_memory_type(
    const VkPhysicalDevice& physical_device,
    const uint32_t& type_filter,
    const VkMemoryPropertyFlags& properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - find_memory_type] Failed to find suitable memory type!");
    throw std::runtime_error("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - find_memory_type] Failed to find suitable memory type!");
}

VkFormat Agos::AgVulkanHandlerColorDepthRessourcesManager::find_depth_format(
    const VkPhysicalDevice& physical_device)
{
    return find_supported_format(physical_device,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat Agos::AgVulkanHandlerColorDepthRessourcesManager::find_supported_format(
    const VkPhysicalDevice& physical_device,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features)
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
    AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - find_supported_format] Failed to find supported format!");
    throw std::runtime_error("[Vulkan/AgVulkanHandlerColorDepthRessourcesManager - find_supported_format] Failed to find supported format!");
}
