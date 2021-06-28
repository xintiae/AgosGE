#include "Agos/src/renderer/vulkan_swapchain.h"

#include "Agos/src/logger/logger.h"
#include <cstdint>


Agos::AgVulkanHandlerSwapChain::AgVulkanHandlerSwapChain()
{
    m_LogicalDeviceReference = VK_NULL_HANDLE;
}

Agos::AgVulkanHandlerSwapChain::AgVulkanHandlerSwapChain(const VkDevice& logical_device)
{
    m_LogicalDeviceReference = logical_device;
}

Agos::AgVulkanHandlerSwapChain::~AgVulkanHandlerSwapChain()
{
}

Agos::AgResult Agos::AgVulkanHandlerSwapChain::create_swap_chain(
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance)
{
    m_LogicalDeviceReference = logical_device->get_device();

    Agos::VulkanPhysicalDevice::AgSwapChainSupportDetails swapchain_support = physical_device->query_swapchain_support(physical_device->get_device(), glfw_instance);

    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapchain_support.formats);
    VkPresentModeKHR presentMode = choose_swap_present_mode(swapchain_support.present_modes);
    VkExtent2D extent = choose_swap_extent(swapchain_support.capabilities, glfw_instance);

    uint32_t imageCount = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 && imageCount > swapchain_support.capabilities.maxImageCount)
    {
        imageCount = swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = glfw_instance->get_surface();

    create_info.minImageCount = imageCount;
    create_info.imageFormat = surfaceFormat.format;
    create_info.imageColorSpace = surfaceFormat.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    Agos::VulkanPhysicalDevice::AgQueueFamilyIndices indices = physical_device->find_queue_families(physical_device->get_device(), glfw_instance);
    uint32_t queueFamilyIndices[] = {indices.graphics_family.value(), indices.present_family.value()};

    if (indices.graphics_family != indices.present_family)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = presentMode;
    create_info.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(logical_device->get_device(), &create_info, nullptr, &m_SwapChain) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerSwapChain - create_swapchain] Failed to create swap chain!");
        return AG_FAILED_TO_CREATE_SWAPCHAIN;
    }

    vkGetSwapchainImagesKHR(logical_device->get_device(), m_SwapChain, &imageCount, nullptr);
    m_SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logical_device->get_device(), m_SwapChain, &imageCount, m_SwapChainImages.data());

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerSwapChain - create_swapchain] Created swap chain!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerSwapChain::create_image_views(const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device)
{
    m_SwapChainImageViews.resize(m_SwapChainImages.size());

    for (uint32_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        m_SwapChainImageViews[i] = create_image_view(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, logical_device->get_device());
    }

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerSwapChain - create_image_views] Created swap chain image views!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerSwapChain::terminate()
{
    if (!m_Terminated)
    {
        for (const VkImageView& it : m_SwapChainImageViews)
        {
            vkDestroyImageView(m_LogicalDeviceReference, it, nullptr);
        }
        vkDestroySwapchainKHR(m_LogicalDeviceReference, m_SwapChain, nullptr);
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

VkSwapchainKHR& Agos::AgVulkanHandlerSwapChain::get_swapchain()
{
    return m_SwapChain;
}

std::vector<VkImage>& Agos::AgVulkanHandlerSwapChain::get_swapchain_images()
{
    return m_SwapChainImages;
}

std::vector<VkImageView>&  Agos::AgVulkanHandlerSwapChain::get_swapchain_image_views()
{
    return m_SwapChainImageViews;
}

VkFormat& Agos::AgVulkanHandlerSwapChain::get_swapchain_image_format()
{
    return m_SwapChainImageFormat;
}

VkExtent2D& Agos::AgVulkanHandlerSwapChain::get_swapchain_extent()
{
    return m_SwapChainExtent;
}

VkSurfaceFormatKHR Agos::AgVulkanHandlerSwapChain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    for (const auto &availableFormat : available_formats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR Agos::AgVulkanHandlerSwapChain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
    for (const auto &availablePresentMode : available_present_modes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Agos::AgVulkanHandlerSwapChain::choose_swap_extent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(glfw_instance->get_window(), &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkImageView Agos::AgVulkanHandlerSwapChain::create_image_view(
    const VkImage& image,
    const VkFormat& format,
    const VkImageAspectFlags& aspectFlags,
    const uint32_t& mipLevels,
    const VkDevice& device)
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
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerSwapChain - create_image_view] Failed to create texture image view!");
        return VK_NULL_HANDLE;
    }

    return imageView;
}
