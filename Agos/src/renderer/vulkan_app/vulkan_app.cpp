#include "Agos/src/renderer/vulkan_app/vulkan_app.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/vulkan_app/vulkan_helpers.h"
#include "Agos/src/renderer/vulkan_app/vulkan_app_helpers.h"

#include <array>


Agos::VulkanHandler::VulkanApp::VulkanApp(std::shared_ptr<GLFWHandler::GLFWInstance>& glfw_instance)
    :
    Agos::VulkanHandler::VulkanBase(glfw_instance),
    m_SwapchainDestroyed                (false),
    m_SwapchainImagesDestroyed          (false),
    m_SwapchainImageViewsDestroyed      (false),
    m_SwapchainFramebuffersDestroyed    (false),
    m_RenderPassDestroyed               (false),
    m_GraphicsPipelineLayoutDestroyed   (false),
    m_GraphicsPipelineCacheDestroyed    (false),
    m_GraphicsPipelineDestroyed         (false),
    m_ColorResDestroyed                 (false),
    m_DepthResDestroyed                 (false),
    m_VulkanAppTerminated               (false)
{
}

Agos::VulkanHandler::VulkanApp::~VulkanApp()
{
    terminate_vulkan_app();
}


// ** init vulkan app ===============================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::init_vulkan_app()
{
    create_swapchain();
    create_swapchain_image_views();
    create_depth_res();
    create_color_res();
    create_render_pass();
    create_swapchain_framebuffers();
    create_graphics_pipelines();

    return AG_SUCCESS;
}

// ** terminate vulkan app ==========================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::terminate_vulkan_app()
{
    if (!m_VulkanAppTerminated)
    {
        terminate_swapchain(false);
        destroy_swapchain_image_views();

        terminate_vulkan_base();

        m_VulkanAppTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}


// ** swapchain recreation ==========================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::recreate_swapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_GLFWInstanceRef->get_window(), &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_GLFWInstanceRef->get_window(), &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(m_LogicalDevice);

    this->terminate_swapchain(true);

    create_swapchain();
    create_swapchain_image_views();
    create_color_res();
    create_depth_res();
    create_render_pass();
    create_swapchain_framebuffers();
    create_graphics_pipelines();

    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::terminate_swapchain(const bool& shall_recreate_swapchain_after /* = false*/)
{
    destroy_swapchain_framebuffers  (shall_recreate_swapchain_after);
    destroy_graphics_pipelines      (shall_recreate_swapchain_after);
    destroy_render_pass             (shall_recreate_swapchain_after);
    destroy_swapchain               (shall_recreate_swapchain_after);

    return AG_SUCCESS;
}
// ** swapchain recreation ==========================================================================================================================

// ** swapchain managment (includes swapchain images, image views and framebuffers) =================================================================

// * = = = = = = = = = = = = = = = = = = = = swapchain = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_swapchain()
{
    Agos::VulkanHandler::VulkanBase::SwapchainSupportDetails swapchain_support = query_swapchain_support(m_PhysicalDevice, m_WindowSurface);

    VkSurfaceFormatKHR  surfaceFormat   = choose_swap_surface_format    (swapchain_support.formats);
    VkPresentModeKHR    presentMode     = choose_swap_present_mode      (swapchain_support.present_modes);
    VkExtent2D          extent          = choose_swap_extent            (swapchain_support.capabilities, m_GLFWInstanceRef->get_window());

    uint32_t imageCount = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 && imageCount > swapchain_support.capabilities.maxImageCount)
    {
        imageCount = swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType               = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface             = m_WindowSurface;
    create_info.minImageCount       = imageCount;
    create_info.imageFormat         = surfaceFormat.format;
    create_info.imageColorSpace     = surfaceFormat.colorSpace;
    create_info.imageExtent         = extent;
    create_info.imageArrayLayers    = 1;
    create_info.imageUsage          = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    Agos::VulkanHandler::VulkanBase::QueueFamiliesIndices indices = find_queue_families_indices(m_PhysicalDevice, m_WindowSurface);
    uint32_t queueFamilyIndices[] = {indices.graphics_family.value(), indices.present_family.value()};

    if (indices.graphics_family != indices.present_family)
    {
        create_info.imageSharingMode        = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount   = 2;
        create_info.pQueueFamilyIndices     = queueFamilyIndices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform    = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha  = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode     = presentMode;
    create_info.clipped         = VK_TRUE;

    if (vkCreateSwapchainKHR(m_LogicalDevice, &create_info, m_Allocator, &m_Swapchain) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL(           "[Vulkan/Agos::VulkanHandler::VulkanApp - create_swapchain] Failed to create swap chain!");
        throw std::runtime_error(   "[Vulkan/Agos::VulkanHandler::VulkanApp - create_swapchain] Failed to create swap chain!");
        return AG_FAILED_TO_CREATE_SWAPCHAIN;
    }

    vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &imageCount, nullptr);
    m_SwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &imageCount, m_SwapchainImages.data());

    m_SwapchainFormat = surfaceFormat.format;
    m_SwapchainExtent = extent;

    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_swapchain] Created swap chain!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_swapchain(const bool& shall_recreate_after /*= false*/)
{
    if (!m_SwapchainDestroyed)
    {
        vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain, m_Allocator);
        m_SwapchainDestroyed = shall_recreate_after;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = swapchain = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = swapchain image views = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_swapchain_image_views()
{
    m_SwapchainImageViews.resize(m_SwapchainImages.size());

    for (uint32_t i = 0; i < m_SwapchainImages.size(); i++)
    {
        m_SwapchainImageViews[i] = Agos::VulkanHandler::VulkanHelpers::create_image_view(m_LogicalDevice, m_SwapchainImages[i], m_SwapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, m_Allocator);
    }

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerSwapChain - create_image_views] Created swap chain image views!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_swapchain_image_views()
{
    if (!m_SwapchainImageViewsDestroyed)
    {
        for (const VkImageView& it : m_SwapchainImageViews)
        {
            vkDestroyImageView(m_LogicalDevice, it, m_Allocator);
        }
        m_SwapchainImageViewsDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = swapchain image views = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = swapchain framebuffers = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_swapchain_framebuffers()
{
    m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());

    for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
    {
        std::array<VkImageView, 3> attachments = {
            m_ColorImageView,
            m_DepthImageView,
            m_SwapchainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = m_RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments    = attachments.data();
        framebufferInfo.width           = m_SwapchainExtent.width;
        framebufferInfo.height          = m_SwapchainExtent.height;
        framebufferInfo.layers          = 1;

        if (vkCreateFramebuffer(m_LogicalDevice, &framebufferInfo, m_Allocator, &m_SwapchainFramebuffers[i]) != VK_SUCCESS)
        {
            AG_CORE_CRITICAL(           "[Vulkan/Agos::VulkanHandler::VulkanApp - create_framebuffers] Failed to create swap chain framebuffers!");
            throw std::runtime_error(   "[Vulkan/Agos::VulkanHandler::VulkanApp - create_framebuffers] Failed to create swap chain framebuffers!");
            return AG_FAILED_TO_CREATE_FRAMEBUFFERS;
        }
    }

    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_framebuffers] Created swap chain framebuffers!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_swapchain_framebuffers(const bool& shall_recreate_after /*= false*/)
{
    if (!m_SwapchainFramebuffersDestroyed)
    {
        m_SwapchainFramebuffersDestroyed = shall_recreate_after;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = swapchain framebuffers = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = swapchain helper functions = = = = = = = = = = = = = = = = = = = =
VkSurfaceFormatKHR Agos::VulkanHandler::VulkanApp::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    for (const VkSurfaceFormatKHR &availableFormat : available_formats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR Agos::VulkanHandler::VulkanApp::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes)
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

VkExtent2D Agos::VulkanHandler::VulkanApp::choose_swap_extent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    GLFWwindow*&                    window)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}
// * = = = = = = = = = = = = = = = = = = = = swapchain helper functions = = = = = = = = = = = = = = = = = = = =

// ** swapchain managment (includes swapchain images, image views and framebuffers) =================================================================


// ** MSAA, Depth and Stencil managment (color and depth res) =======================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_color_res()
{
    VkFormat colorFormat = m_SwapchainFormat;

    auto [m_ColorImage, m_ColorImageMemory] = Agos::VulkanHandler::VulkanHelpers::create_image(
        m_PhysicalDevice,
        m_LogicalDevice,
        m_SwapchainExtent.width,
        m_SwapchainExtent.height,
        1,
        m_MsaaSamples,
        colorFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_Allocator);
    
    m_ColorImageView = Agos::VulkanHandler::VulkanHelpers::create_image_view(
        m_LogicalDevice, m_ColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, m_Allocator);

    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_color_ressources] Successfully created color ressources for msaa!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_color_res(const bool& shall_recreate_after /*= false*/)
{
    if (!m_ColorResDestroyed)
    {
        vkDestroyImageView  (m_LogicalDevice, m_ColorImageView,     m_Allocator);
        vkDestroyImage      (m_LogicalDevice, m_ColorImage,         m_Allocator);
        vkFreeMemory        (m_LogicalDevice, m_ColorImageMemory,   m_Allocator);
        m_ColorResDestroyed = shall_recreate_after;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::create_depth_res()
{
    VkFormat depthFormat = Agos::VulkanHandler::VulkanHelpers::find_depth_format(m_PhysicalDevice);

    auto [m_DepthImage, m_DepthImageMemory] = Agos::VulkanHandler::VulkanHelpers::create_image(
        m_PhysicalDevice,
        m_LogicalDevice,
        m_SwapchainExtent.width,
        m_SwapchainExtent.height,
        1,
        m_MsaaSamples,
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_Allocator
    );

    m_DepthImageView = Agos::VulkanHandler::VulkanHelpers::create_image_view(
        m_LogicalDevice, m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, m_Allocator);

    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_depth_ressources] Successfully created depth ressources for msaa!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_depth_res(const bool& shall_recreate_after /*= false*/)
{
    if (!m_DepthResDestroyed)
    {
        vkDestroyImageView  (m_LogicalDevice, m_DepthImageView,     m_Allocator);
        vkDestroyImage      (m_LogicalDevice, m_DepthImage,         m_Allocator);
        vkFreeMemory        (m_LogicalDevice, m_DepthImageMemory,   m_Allocator);
        m_DepthResDestroyed = shall_recreate_after;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// ** MSAA, Depth and Stencil managment (color and depth res) =======================================================================================


// ** Render pass managment =========================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_render_pass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format                  = m_SwapchainFormat;
    colorAttachment.samples                 = m_MsaaSamples;
    colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format                  = find_depth_format(m_PhysicalDevice);
    depthAttachment.samples                 = m_MsaaSamples;
    depthAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format           = m_SwapchainFormat;
    colorAttachmentResolve.samples          = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment           = 0;
    colorAttachmentRef.layout               = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment           = 1;
    depthAttachmentRef.layout               = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment    = 2;
    colorAttachmentResolveRef.layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint               = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount            = 1;
    subpass.pColorAttachments               = &colorAttachmentRef;
    subpass.pDepthStencilAttachment         = &depthAttachmentRef;
    subpass.pResolveAttachments             = &colorAttachmentResolveRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass                   = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass                   = 0;
    dependency.srcStageMask                 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask                = 0;
    dependency.dstStageMask                 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask                = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount        = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments           = attachments.data();
    render_pass_info.subpassCount           = 1;
    render_pass_info.pSubpasses             = &subpass;
    render_pass_info.dependencyCount        = 1;
    render_pass_info.pDependencies          = &dependency;

    if (vkCreateRenderPass(m_LogicalDevice, &render_pass_info, m_Allocator, &m_RenderPass) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL(           "[Vulkan/Agos::VulkanHandler::VulkanApp - create_render_pass] Failed to create render pass!");
        throw std::runtime_error(   "[Vulkan/Agos::VulkanHandler::VulkanApp - create_render_pass] Failed to create render pass!");
        return AG_FAILED_TO_CREATE_RENDER_PASS;
    }

    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_render_pass] Created render pass!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_render_pass(const bool& shall_recreate_after /*= false*/)
{
    if (!m_RenderPassDestroyed)
    {
        vkDestroyRenderPass(m_LogicalDevice, m_RenderPass, m_Allocator);
        m_RenderPassDestroyed = shall_recreate_after;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

// * = = = = = = = = = = = = = = = = = = = = color and depth res helper functions = = = = = = = = = = = = = = = = = = = =
VkFormat Agos::VulkanHandler::VulkanApp::find_depth_format(const VkPhysicalDevice& physical_device)
{
    return find_supported_format(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        physical_device);
}

VkFormat Agos::VulkanHandler::VulkanApp::find_supported_format(
    const std::vector<VkFormat> &candidates,
    const VkImageTiling& tiling,
    const VkFormatFeatureFlags& features,
    const VkPhysicalDevice& physical_device)
{
    for (const VkFormat& format : candidates)
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

    throw std::runtime_error("[Vulkan/Agos::VulkanHandler::VulkanApp - find_supported_format] Failed to find supported format!");
}
// * = = = = = = = = = = = = = = = = = = = = color and depth res helper functions = = = = = = = = = = = = = = = = = = = =

// ** Render pass managment =========================================================================================================================


// ** Descriptors managment (descriptor layout, pool and sets) ======================================================================================
// * = = = = = = = = = = = = = = = = = = = = descriptor sets layout = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_descriptor_sets_layout()
{
    // MVP + EL Layout Bindings ============================================================
    std::array<VkDescriptorSetLayoutBinding, 2> MVP_EL_LayoutBindings;
    // MVP binding
    MVP_EL_LayoutBindings[0].binding            = 0;
    MVP_EL_LayoutBindings[0].descriptorCount    = 1;
    MVP_EL_LayoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    MVP_EL_LayoutBindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
    MVP_EL_LayoutBindings[0].pImmutableSamplers = nullptr;
    // EL binding
    MVP_EL_LayoutBindings[1].binding            = 0;
    MVP_EL_LayoutBindings[1].descriptorCount    = 1;
    MVP_EL_LayoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    MVP_EL_LayoutBindings[1].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    MVP_EL_LayoutBindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo DSL_MVP_EL_CreateInfo{};
    DSL_MVP_EL_CreateInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DSL_MVP_EL_CreateInfo.pNext                 = nullptr;
    DSL_MVP_EL_CreateInfo.flags                 = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    DSL_MVP_EL_CreateInfo.bindingCount          = MVP_EL_LayoutBindings.size();    // MVP -uniform buffer- + EL -uniform buffer-
    DSL_MVP_EL_CreateInfo.pBindings             = MVP_EL_LayoutBindings.data();

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &DSL_MVP_EL_CreateInfo, m_Allocator, &m_DSL_MVP_EL) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanApp - create_descriptors_set_layout] Failed to create MVP + EL descriptor sets layout!");
        return AG_FAILED_TO_CREATE_DESCRIPTOR_SET_LAYOUT;
    }
    // MVP + EL Layout Bindings ============================================================


    // Material Layout Bindings ============================================================
    VkDescriptorSetLayoutBinding materialLayoutBinding{};
    materialLayoutBinding.binding               = 1;
    materialLayoutBinding.descriptorCount       = 1;
    materialLayoutBinding.descriptorType        = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    materialLayoutBinding.stageFlags            = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBinding.pImmutableSamplers    = nullptr;

    VkDescriptorSetLayoutCreateInfo DSL_Materials_CreateInfo{};
    DSL_Materials_CreateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DSL_Materials_CreateInfo.pNext              = nullptr;
    DSL_Materials_CreateInfo.flags              = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    DSL_Materials_CreateInfo.bindingCount       = 1;
    DSL_Materials_CreateInfo.pBindings          = &materialLayoutBinding;

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &DSL_Materials_CreateInfo, m_Allocator, &m_DSL_Material) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanApp - create_descriptors_set_layout] Failed to create Material descriptor set layout!");
        return AG_FAILED_TO_CREATE_DESCRIPTOR_SET_LAYOUT;
    }
    // Material Layout Bindings ============================================================


    // Texture + Ambiant + Diffuse + Specular + Normal Maps Layout Bindings ================
    std::array<VkDescriptorSetLayoutBinding, 5> TexAmbDiffSpecNrmlMaps_LayoutBindings{};
    // Texture binding
    TexAmbDiffSpecNrmlMaps_LayoutBindings[0].binding            = 0;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[0].descriptorCount    = 1;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[0].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[0].pImmutableSamplers = nullptr;
    // Ambiant binding
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].binding            = 1;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].descriptorCount    = 1;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].pImmutableSamplers = nullptr;
    // Diffuse binding
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].binding            = 2;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].descriptorCount    = 1;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].pImmutableSamplers = nullptr;
    // Specular binding
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].binding            = 3;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].descriptorCount    = 1;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].pImmutableSamplers = nullptr;
    // Ambiant binding
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].binding            = 4;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].descriptorCount    = 1;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo DSL_TexAmbDiffSpecNrmlMaps_CreateInfo{};
    DSL_TexAmbDiffSpecNrmlMaps_CreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DSL_TexAmbDiffSpecNrmlMaps_CreateInfo.pNext         = nullptr;
    DSL_TexAmbDiffSpecNrmlMaps_CreateInfo.flags         = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    DSL_TexAmbDiffSpecNrmlMaps_CreateInfo.bindingCount  = TexAmbDiffSpecNrmlMaps_LayoutBindings.size();
    DSL_TexAmbDiffSpecNrmlMaps_CreateInfo.pBindings     = TexAmbDiffSpecNrmlMaps_LayoutBindings.data();

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &DSL_TexAmbDiffSpecNrmlMaps_CreateInfo, m_Allocator, &m_DSL_TexAmbDiffSpecNrmlMaps) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanApp - create_descriptors_set_layout] Failed to create TexAmbDiffSpecNrmlMaps descriptor sets layout!");
        return AG_FAILED_TO_CREATE_DESCRIPTOR_SET_LAYOUT;
    }
    // Texture + Ambiant + Diffuse + Specular + Normal Maps Layout Bindings ================


    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_decriptor_sets_layout()
{
    if (!m_DescriptorSetLayoutDestroyed)
    {
        vkDestroyDescriptorSetLayout(m_LogicalDevice, m_DSL_MVP_EL                  , m_Allocator);
        vkDestroyDescriptorSetLayout(m_LogicalDevice, m_DSL_Material                , m_Allocator);
        vkDestroyDescriptorSetLayout(m_LogicalDevice, m_DSL_TexAmbDiffSpecNrmlMaps  , m_Allocator);

        m_DescriptorSetLayoutDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = descriptor sets layout = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = descriptor pool = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_descriptor_pool()
{
    std::array<VkDescriptorPoolSize, 7> poolSizes;
    // MVP
    poolSizes[0].type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount    = 100;
    // EL
    poolSizes[1].type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount    = 100;
    // Material
    poolSizes[2].type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount    = 100;
    // Tex Sampler
    poolSizes[3].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount    = 100;
    // Amb Sampler
    poolSizes[4].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount    = 100;
    // Diff Sampler
    poolSizes[5].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[5].descriptorCount    = 100;
    // Spec Sampler
    poolSizes[6].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[6].descriptorCount    = 100;
    // Normal Sampler
    poolSizes[7].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[7].descriptorCount    = 100;
    

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext            = nullptr;
    createInfo.flags            = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    createInfo.poolSizeCount    = poolSizes.size();
    createInfo.pPoolSizes       = poolSizes.data();
    createInfo.maxSets          = AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS;

    if (vkCreateDescriptorPool(m_LogicalDevice, &createInfo, m_Allocator, &m_DescriptorPool) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL(           "[Vulkan/VulkanHandler::VulkanApp - create_descriptor_pool] Failed to create descriptor pool!");
        return AG_FAILED_TO_CREATE_DESCRIPTOR_POOL;

        // useless since we exit the function before throwing a runtime error 
        // but Vulkan won't get any further since it can't do anything else but crash without this
        throw std::runtime_error(   "[Vulkan/VulkanHandler::VulkanApp - create_descriptor_pool] Failed to create descriptor pool!");
    }

    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_descriptor_pool(const bool& shall_recreate_after /* = false*/)
{
    if (!m_DescriptorPoolDestroyed)
    {
        vkDestroyDescriptorPool(m_LogicalDevice, m_DescriptorPool, m_Allocator);
        m_DescriptorPoolDestroyed = shall_recreate_after;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = descriptor pool = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = descriptor sets = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_descriptor_sets()
{
    for (size_t i = 0; i < m_ToRenderModels.size(); i++)
    {
        // m_DS_MVP_EL
        std::vector<VkDescriptorSetLayout> layouts (m_SwapchainImages.size(), m_DSL_MVP_EL);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.descriptorPool        = m_DescriptorPool;
        allocInfo.descriptorSetCount    = static_cast<uint32_t>(layouts.size());
        allocInfo.pSetLayouts           = layouts.data();

        m_DS_MVP_EL[i].resize(m_SwapchainImages.size());

        if (vkAllocateDescriptorSets(m_LogicalDevice, &allocInfo, m_DS_MVP_EL[i].data()) != VK_SUCCESS)
        {
            AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_descriptor_sets] Failed to allocate MVP EL descriptor sets!");
            return AG_FAILED_TO_ALLOCATE_DESCRIPTOR_SETS;
        }

        return AG_SUCCESS;

    }
}
// * = = = = = = = = = = = = = = = = = = = = descriptor sets = = = = = = = = = = = = = = = = = = = =
// ** Descriptors managment (descriptor layout, pool and sets) ======================================================================================


// ** Graphics Pipeline managment ===================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_graphics_pipelines()
{
    std::array<VkShaderModule, 2> vertShaderModule = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::create_shader_module(
        m_LogicalDevice, std::string(AG_SHADERS_PATH) + std::string(AG_DEFAULT_VERTEX_SHADER_FOLDER), m_Allocator);
    std::array<VkShaderModule, 2> fragShaderModule = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::create_shader_module(
        m_LogicalDevice, std::string(AG_SHADERS_PATH) + std::string(AG_DEFAULT_FRAGMENT_SHADER_FOLDER), m_Allocator);

    // vertex generic
    VkPipelineShaderStageCreateInfo genericVertShaderStageInfo{};
    vertShaderStageInfo.sType                       = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage                       = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module                      = vertShaderModule;
    vertShaderStageInfo.pName                       = "main";

    // vertex lighting
    VkPipelineShaderStageCreateInfo lightingVertShaderStageInfo{};

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType                       = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage                       = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module                      = fragShaderModule;
    fragShaderStageInfo.pName                       = "main";

    VkPipelineShaderStageCreateInfo shaderStages[]  = {vertShaderStageInfo, fragShaderStageInfo};


    VkVertexInputBindingDescription bindingDescription                      = Agos::VulkanHandler::VulkanModeling::Vertex::get_binding_description();
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions  = Agos::VulkanHandler::VulkanModeling::Vertex::get_attribute_description();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();


    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                             = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology                          = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable            = VK_FALSE;


    VkViewport viewport{};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = static_cast<float>(m_SwapchainExtent.width);
    viewport.height     = static_cast<float>(m_SwapchainExtent.height);
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor{};
    scissor.offset      = {0, 0};
    scissor.extent      = m_SwapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType                 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount         = 1;
    viewportState.pViewports            = &viewport;
    viewportState.scissorCount          = 1;
    viewportState.pScissors             = &scissor;


    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable         = VK_FALSE;
    rasterizer.rasterizerDiscardEnable  = VK_FALSE;
    rasterizer.polygonMode              = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth                = 1.0f;
    rasterizer.cullMode                 = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace                = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable          = VK_FALSE;


    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = m_MsaaSamples;


    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable        = VK_TRUE;
    depthStencil.depthWriteEnable       = VK_TRUE;
    depthStencil.depthCompareOp         = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable  = VK_FALSE;
    depthStencil.stencilTestEnable      = VK_FALSE;


    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable    = VK_FALSE;


    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType                 = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable         = VK_FALSE;
    colorBlending.logicOp               = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount       = 1;
    colorBlending.pAttachments          = &colorBlendAttachment;
    colorBlending.blendConstants[0]     = 0.0f;
    colorBlending.blendConstants[1]     = 0.0f;
    colorBlending.blendConstants[2]     = 0.0f;
    colorBlending.blendConstants[3]     = 0.0f;


    std::array<VkDescriptorSetLayout, 3> DSLs = { m_DSL_MVP_EL, m_DSL_Material, m_DSL_TexAmbDiffSpecNrmlMaps };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount   = DSLs.size();
    pipelineLayoutInfo.pSetLayouts      = DSLs.data();


    if (vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayoutInfo, m_Allocator, &m_GraphicsPipelineLayout) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerGraphicsPipelineManger - create_graphics_pipeline] Failed to create graphics pipeline layout!");
        return AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE_LAYOUT;
    }


    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount             = 2;
    pipelineInfo.pStages                = shaderStages;
    pipelineInfo.pVertexInputState      = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState    = &inputAssembly;
    pipelineInfo.pViewportState         = &viewportState;
    pipelineInfo.pRasterizationState    = &rasterizer;
    pipelineInfo.pMultisampleState      = &multisampling;
    pipelineInfo.pDepthStencilState     = &depthStencil;
    pipelineInfo.pColorBlendState       = &colorBlending;
    pipelineInfo.layout                 = m_GraphicsPipelineLayout;
    pipelineInfo.renderPass             = m_RenderPass;
    pipelineInfo.subpass                = 0;
    pipelineInfo.basePipelineHandle     = VK_NULL_HANDLE;


    if (vkCreateGraphicsPipelines(m_LogicalDevice, m_GraphicsPipelineCache, 1, &pipelineInfo, m_Allocator, &m_GraphicsPipeline) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanApp - create_graphics_pipeline] Failed to create graphics pipeline!");
        return AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE;
    }

    vkDestroyShaderModule(m_LogicalDevice, fragShaderModule, m_Allocator);
    vkDestroyShaderModule(m_LogicalDevice, vertShaderModule, m_Allocator);
    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_graphics_pipeline] created graphics pipeline!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_graphics_pipelines(const bool& shall_recreate_after /*= false*/)
{
    if (!m_GraphicsPipelineDestroyed)
    {
        vkDestroyPipeline(m_LogicalDevice, m_GraphicsPipeline, m_Allocator);
        m_GraphicsPipelineDestroyed = shall_recreate_after;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// ** Graphics Pipeline managment ===================================================================================================================

