#include "Agos/src/renderer/vulkan_app/vulkan_app.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/vulkan_app/vulkan_helpers.h"
#include "Agos/src/renderer/vulkan_app/vulkan_app_helpers.h"

#include AG_GLM_INCLUDE
#include <cstdlib>  // std::div

Agos::VulkanHandler::VulkanApp::VulkanApp(std::shared_ptr<GLFWHandler::GLFWInstance>& glfw_instance)
    :
    Agos::VulkanHandler::VulkanBase(glfw_instance),
    m_SwapchainDestroyed                (false),
    m_SwapchainImagesDestroyed          (false),
    m_SwapchainImageViewsDestroyed      (false),
    m_SwapchainFramebuffersDestroyed    (false),

    m_RenderPassDestroyed               (false),

    m_OffscreenRenderPassDestroyed      (false),
    m_OffscreenImageResDestroyed        (false),
    m_OffscreenFramebufferDestoyed      (false),
    m_OffscreenSamplerDestroyed         (false),
    m_ColorResDestroyed                 (false),
    m_DepthResDestroyed                 (false),

    m_GraphicsPipelineLayoutDestroyed   (false),
    m_GraphicsPipelineCacheDestroyed    (false),
    m_GraphicsPipelineDestroyed         (false),

    m_DescriptorSetLayoutDestroyed      (false),
    m_DescriptorPoolDestroyed           (false),
    m_UniformBuffersDestroyed           (false),
    m_EntitiesVertexBuffersDestroyed    (false),
    m_EntitiesIndexBufferDestroyed      (false),
    m_EntitiesTexturesUnloaded          (false),

    m_CommandBuffersDestroyed           (false),
    m_CommandBuffersPoolDestroyed       (false),
    m_SyncObjectsDestroyed              (false),

    m_OffscreenRenderTargetDestroyed    (false),
    m_SwapchainAlreadyTerminated        (false),
    m_VulkanAppTerminated               (false)
{
    m_ImGuiInterface    = std::make_unique<Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance>();
    m_GLFWInterfaceRef->get_listener()->listen<Agos::GLFWHandler::GLFWEvent::Event>(
        [this](const Agos::GLFWHandler::GLFWEvent::Event& event) -> void
        {
            this->process_events(event);
        }
    );
}

Agos::VulkanHandler::VulkanApp::~VulkanApp()
{
    terminate_vulkan_app();
}

// ** init vulkan app ===============================================================================================================================
// * = * = * = * = * VulkanApp * = * = * = * = *
Agos::AgResult Agos::VulkanHandler::VulkanApp::init_vulkan_app()
{
    // --------------------------------
    setup_vulkan_base               ();
    // --------------------------------
    create_swapchain                ();
    create_swapchain_image_views    ();
    create_command_buffers_pool     ();
    create_descriptor_pool          ();
    create_uniform_buffers          ();
    create_descriptor_sets_layout   ();
    create_sync_objects             ();
    // --------------------------------
    create_offscreen_render_target  ();
    // --------------------------------
    create_imgui_render_target      ();
    init_vulkan_imgui               ();
    // --------------------------------
    return AG_SUCCESS;
}
// * = * = * = * = * VulkanApp * = * = * = * = *

// * = * = * = * = * ImGui * = * = * = * = *
Agos::AgResult Agos::VulkanHandler::VulkanApp::init_vulkan_imgui()
{
    Agos::VulkanHandler::VulkanBase::SwapchainSupportDetails swapchainSupport = Agos::VulkanHandler::VulkanBase::query_swapchain_support(
        m_PhysicalDevice,
        m_WindowSurface
    );

    // we "just" have to fill in initInfo struct
    Agos::ImGuiHandler::ImGuiVulkan::ImGui_initInfo imgui_initInfo{};
    imgui_initInfo.Window               = m_GLFWInterfaceRef->get_window();
    imgui_initInfo.Instance             = m_Instance;
    imgui_initInfo.PhysicalDevice       = m_PhysicalDevice;
    imgui_initInfo.Device               = &m_LogicalDevice;
    imgui_initInfo.QueueFamily          = Agos::VulkanHandler::VulkanBase::find_queue_families_indices(m_PhysicalDevice, m_WindowSurface).graphics_family.value();
    imgui_initInfo.Queue                = m_GraphicsQueue;
    imgui_initInfo.PipelineCache        = VK_NULL_HANDLE;
    imgui_initInfo.RenderPass           = m_RenderPass;
    // Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance will provide one
    // imgui_initInfo.DescriptorPool       = m_DescriptorPool;
    imgui_initInfo.CommandPool          = m_CommandBuffersPool;
    imgui_initInfo.Subpass              = 0;
    imgui_initInfo.MinImageCount        = swapchainSupport.capabilities.minImageCount;
    imgui_initInfo.ImageCount           = m_SwapchainImages.size();
    imgui_initInfo.MSAASamples          = VK_SAMPLE_COUNT_1_BIT;
    imgui_initInfo.Allocator            = m_Allocator;
    imgui_initInfo.CheckVkResultFn      = Agos::ImGuiHandler::ImGuiVulkan::ag_vulkan_imgui_check_err;

    m_ImGuiInterface->init(&imgui_initInfo);

    m_OffscreenImGuiID = ImGui_ImplVulkan_AddTexture(m_OffscreenSampler, m_OffscreenImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    return AG_SUCCESS;
}
// * = * = * = * = * ImGui * = * = * = * = *
// ** init vulkan app ===============================================================================================================================

// ** load, update and unload entities ==============================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::load_entities(const std::vector<std::shared_ptr<Agos::Entities::Entity>>& entities_to_render)
{
    m_CurrentFrame = 0;
    m_ToRenderEntities = std::move(entities_to_render);
    for (std::shared_ptr<Agos::Entities::Entity>& it : m_ToRenderEntities)
    {
        it->get_entity_gpu_status() = true;
    }

    load_entities_textures  ();
    create_vertex_buffers   ();
    create_index_buffers    ();
    create_descriptor_sets  ();

    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::query_scene_state(
    const std::shared_ptr<Agos::SceneManager::SceneStatus>& scene_status
)
{
    m_SceneState = std::move(scene_status);
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::unload_entities()
{
    recreate_swapchain();
    unload_entities_textures();

    for (std::shared_ptr<Agos::Entities::Entity>& it : m_ToRenderEntities)
    {
        it->get_entity_gpu_status() = false;
    }

    return AG_SUCCESS;
}
// ** load, update and unload entities ==============================================================================================================

// ** terminate vulkan app ==========================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::terminate_vulkan_app()
{
    if (!m_VulkanAppTerminated)
    {
        // --------------------------------
        terminate_swapchain             ();
        // --------------------------------
        m_ImGuiInterface->terminate     ();
        // --------------------------------
        destroy_offscreen_render_target ();
        // --------------------------------
        destroy_sync_objects            ();

        destroy_vertex_buffers          ();
        destroy_index_buffers           ();
        destroy_uniform_buffers         ();

        destroy_descriptor_pool         ();
        destroy_command_buffers_pool    ();
        destroy_decriptor_sets_layout   ();
        destroy_swapchain_image_views   ();
        // --------------------------------
        terminate_vulkan_base           ();
        // --------------------------------

        m_VulkanAppTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// ** terminate vulkan app ==========================================================================================================================

// ** Draw Calls ====================================================================================================================================
// * = = = = = = = = = = = = = = = = = = = = draw_frame = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::draw_frame()
{
    update_entities();
    uint32_t imageIndex = acquire_next_swapchain_image();
    draw_imgui_objects              ();
    // update_vertex_buffers           ();
    // update_index_buffers            ();
    update_uniform_buffers          ();
    record_command_buffers          (imageIndex);
    submit_image_for_presentation   (imageIndex);
    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = draw_frame = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = update_entities = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::update_entities()
{
    for (size_t i = 0; i < m_ToRenderEntities.size(); i++)
    {
        if ( m_ToRenderEntities[i]->entity_destroyed() || !(m_ToRenderEntities[i]->should_be_shown()) )
        {
            m_ToRenderEntities.erase(m_ToRenderEntities.begin() + i);
        }
    }

    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = update_entities = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = image acquisition, submition and presentation = = = = = = = = = = = = = = = = = = = =
uint32_t Agos::VulkanHandler::VulkanApp::acquire_next_swapchain_image()
{
    uint32_t imageIndex;

    vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
    VkResult result = vkAcquireNextImageKHR(m_LogicalDevice, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

    if ( result == VK_ERROR_OUT_OF_DATE_KHR )
    {
        recreate_swapchain();
        return acquire_next_swapchain_image();
    }
    else if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
    {
        AG_CORE_CRITICAL        ("[Vulkan/VulkanHandler::VulkanApp - draw_frame] Failed to acquire next swap chain image!");
        throw std::runtime_error("[Vulkan/VulkanHandler::VulkanApp - draw_frame] Failed to acquire next swap chain image!");
    }

    return imageIndex;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::submit_image_for_presentation(const uint32_t& image_index)
{
    if ( m_ImagesInFlight[image_index] != VK_NULL_HANDLE )
    {
        vkWaitForFences(m_LogicalDevice, 1, &m_ImagesInFlight[image_index], VK_TRUE, UINT64_MAX);
    }

    m_ImagesInFlight[image_index] = m_InFlightFences[m_CurrentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore             waitSemaphores[]    = { m_ImageAvailableSemaphores[m_CurrentFrame]      };
    VkPipelineStageFlags    waitStages[]        = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT   };

    submitInfo.waitSemaphoreCount       = 1;
    submitInfo.pWaitSemaphores          = waitSemaphores;
    submitInfo.pWaitDstStageMask        = waitStages;
    submitInfo.commandBufferCount       = 1;
    submitInfo.pCommandBuffers          = &m_CommandBuffers[image_index];

    VkSemaphore signalSemaphores[]      = { m_RenderFinishedSemaphores[m_CurrentFrame] };
    submitInfo.signalSemaphoreCount     = 1;
    submitInfo.pSignalSemaphores        = signalSemaphores;

    vkResetFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame]);

    VkResult result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);

    // if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResizedFlag )
    if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        m_FramebufferResizedFlag = false;
        recreate_swapchain();
    }
    else if ( result != VK_SUCCESS )
    {
        AG_CORE_CRITICAL        ("[Vulkan/VulkanHandler::VulkanApp - draw_frame] Failed to submit draw command buffer!");
        throw std::runtime_error("[Vulkan/VulkanHandler::VulkanApp - draw_frame] Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount  = 1;
    presentInfo.pWaitSemaphores     = signalSemaphores;
    presentInfo.swapchainCount      = 1;
    presentInfo.pSwapchains         = &m_Swapchain;
    presentInfo.pImageIndices       = &image_index;

    vkQueuePresentKHR(m_PresentQueue, &presentInfo);

    m_CurrentFrame = (m_CurrentFrame + 1) % AG_VULKAN_MAX_FRAMES_IN_FLIGHT;

    return AG_SUCCESS;
}
// ** Draw Calls ====================================================================================================================================


// ** Swapchain Recreation ==========================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::recreate_swapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_GLFWInterfaceRef->get_window(), &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_GLFWInterfaceRef->get_window(), &width, &height);
        glfwWaitEvents();
    }
    this->terminate_swapchain();

    this->create_swapchain();
    this->create_swapchain_image_views();
    this->create_render_pass();
    this->create_swapchain_framebuffers();

    m_SwapchainAlreadyTerminated = false;

    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::terminate_swapchain()
{
    if (!m_SwapchainAlreadyTerminated)
    {
        vkDeviceWaitIdle(m_LogicalDevice);
        destroy_swapchain_framebuffers  ();
        destroy_command_buffers         ();
        destroy_render_pass             ();
        destroy_swapchain_image_views   ();
        destroy_swapchain               ();

        m_SwapchainAlreadyTerminated = true;
        return AG_INSTANCE_ALREADY_TERMINATED;
    }
    vkDeviceWaitIdle(m_LogicalDevice);
    return AG_SUCCESS;
}
// ** Swapchain Recreation ==========================================================================================================================

// ** Swapchain Managment (includes swapchain images, image views and framebuffers) =================================================================
// * = = = = = = = = = = = = = = = = = = = = swapchain = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_swapchain()
{
    Agos::VulkanHandler::VulkanBase::SwapchainSupportDetails swapchain_support = query_swapchain_support(m_PhysicalDevice, m_WindowSurface);

    VkSurfaceFormatKHR  surfaceFormat   = choose_swap_surface_format    (swapchain_support.formats);
    VkPresentModeKHR    presentMode     = choose_swap_present_mode      (swapchain_support.present_modes);
    VkExtent2D          extent          = choose_swap_extent            (swapchain_support.capabilities, m_GLFWInterfaceRef->get_window());

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

    m_SwapchainDestroyed = false;
    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_swapchain] Created swap chain!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_swapchain()
{
    if (!m_SwapchainDestroyed)
    {
        vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain, m_Allocator);
        m_SwapchainDestroyed = true;
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

    m_SwapchainImageViewsDestroyed = false;
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
        std::array<VkImageView, 1> attachments = {
            m_SwapchainImageViews[i]
        };

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

    m_SwapchainFramebuffersDestroyed = false;
    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_framebuffers] Created swap chain framebuffers!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_swapchain_framebuffers()
{
    if (!m_SwapchainFramebuffersDestroyed)
    {
        for (const VkFramebuffer& it : m_SwapchainFramebuffers)
        {
            vkDestroyFramebuffer(m_LogicalDevice, it, m_Allocator);
        }
        m_SwapchainFramebuffers.clear();
        m_SwapchainFramebuffersDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = swapchain framebuffers = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = swapchain helpers = = = = = = = = = = = = = = = = = = = =
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
    GLFWwindow*                    window
)
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
// * = = = = = = = = = = = = = = = = = = = = swapchain helpers = = = = = = = = = = = = = = = = = = = =
// ** Swapchain Managment (includes swapchain images, image views and framebuffers) =================================================================


// ** MSAA, Depth and Stencil managment (color and depth res) =======================================================================================
// * = = = = = = = = = = = = = = = = = = = = color and depth resources = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_color_res()
{
    VkFormat colorFormat = m_OffscreenFormat;

    Agos::VulkanHandler::VulkanHelpers::AgImage colorImage = Agos::VulkanHandler::VulkanHelpers::create_image(
        m_PhysicalDevice,
        m_LogicalDevice,
        m_OffscreenExtent.width,
        m_OffscreenExtent.height,
        1,
        m_MsaaSamples,
        colorFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_Allocator);
    
    m_ColorImage        = colorImage.image;
    m_ColorImageMemory  = colorImage.imageMemory;
    
    m_ColorImageView = Agos::VulkanHandler::VulkanHelpers::create_image_view(
        m_LogicalDevice, m_ColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, m_Allocator);

    m_ColorResDestroyed = false;
    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_color_ressources] Successfully created color ressources for msaa!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_color_res()
{
    if (!m_ColorResDestroyed)
    {
        vkDestroyImageView  (m_LogicalDevice, m_ColorImageView,     m_Allocator);
        vkDestroyImage      (m_LogicalDevice, m_ColorImage,         m_Allocator);
        vkFreeMemory        (m_LogicalDevice, m_ColorImageMemory,   m_Allocator);
        m_ColorResDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::create_depth_res()
{
    VkFormat depthFormat = Agos::VulkanHandler::VulkanHelpers::find_depth_format(m_PhysicalDevice);

    Agos::VulkanHandler::VulkanHelpers::AgImage depthImage = Agos::VulkanHandler::VulkanHelpers::create_image(
        m_PhysicalDevice,
        m_LogicalDevice,
        m_OffscreenExtent.width,
        m_OffscreenExtent.height,
        1,
        m_MsaaSamples,
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_Allocator
    );
    m_DepthImage        = depthImage.image;
    m_DepthImageMemory  = depthImage.imageMemory;

    m_DepthImageView = Agos::VulkanHandler::VulkanHelpers::create_image_view(
        m_LogicalDevice, m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, m_Allocator);

/*
    Agos::VulkanHandler::VulkanHelpers::transition_image_layout(
        m_LogicalDevice,
        m_GraphicsQueue,
        m_CommandBuffersPool,
        m_DepthImage,
        depthFormat,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        1
    );
*/

    m_DepthResDestroyed = false
    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_depth_ressources] Successfully created depth ressources for msaa!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_depth_res()
{
    if (!m_DepthResDestroyed)
    {
        vkDestroyImageView  (m_LogicalDevice, m_DepthImageView,     m_Allocator);
        vkDestroyImage      (m_LogicalDevice, m_DepthImage,         m_Allocator);
        vkFreeMemory        (m_LogicalDevice, m_DepthImageMemory,   m_Allocator);
        m_DepthResDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = color and depth resources = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = color and depth res helpers = = = = = = = = = = = = = = = = = = = =
VkFormat Agos::VulkanHandler::VulkanApp::find_depth_format(const VkPhysicalDevice& physical_device)
{
    return find_supported_format(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        physical_device);
}

VkFormat Agos::VulkanHandler::VulkanApp::find_supported_format(
    const std::vector<VkFormat>&    candidates,
    const VkImageTiling&            tiling,
    const VkFormatFeatureFlags&     features,
    const VkPhysicalDevice&         physical_device
)
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
// * = = = = = = = = = = = = = = = = = = = = color and depth res helpers = = = = = = = = = = = = = = = = = = = =
// ** MSAA, Depth and Stencil managment (color and depth res) =======================================================================================


// ** Render pass managment =========================================================================================================================
// * = = = = = = = = = = = = = = = = = = = = render pass = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_render_pass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format          = m_SwapchainFormat;
    colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR; // VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;    // shader_read
    colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment             = 0;
    colorRef.layout                 = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorRef;

    std::array<VkSubpassDependency, 1> dependencies({});
    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass      = 0;
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // ? or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

/*
    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass      = 0;
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass      = 0;
    dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
*/

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext            = nullptr;
    renderPassInfo.attachmentCount  = 1;
    renderPassInfo.pAttachments     = &colorAttachment;
    renderPassInfo.subpassCount     = 1;
    renderPassInfo.pSubpasses       = &subpass;
    renderPassInfo.dependencyCount  = dependencies.size();
    renderPassInfo.pDependencies    = dependencies.data();

    if (vkCreateRenderPass(m_LogicalDevice, &renderPassInfo, m_Allocator, &m_RenderPass) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_render_pass] Failed to create render pass!");
        return AG_FAILED_TO_CREATE_RENDER_PASS;
    }

    m_RenderPassDestroyed = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_render_pass()
{
    if (!m_RenderPassDestroyed)
    {
        vkDestroyRenderPass(m_LogicalDevice, m_RenderPass, m_Allocator);
        m_RenderPassDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = render pass = = = = = = = = = = = = = = = = = = = =
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
    MVP_EL_LayoutBindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;    // TODO N 4
    MVP_EL_LayoutBindings[0].pImmutableSamplers = nullptr;
    // EL binding
    MVP_EL_LayoutBindings[1].binding            = 1;
    MVP_EL_LayoutBindings[1].descriptorCount    = 1;
    MVP_EL_LayoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    MVP_EL_LayoutBindings[1].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    MVP_EL_LayoutBindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo DSL_MVP_EL_CreateInfo{};
    DSL_MVP_EL_CreateInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DSL_MVP_EL_CreateInfo.pNext                 = nullptr;
    // DSL_MVP_EL_CreateInfo.flags                 = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
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
    materialLayoutBinding.binding               = 0;
    materialLayoutBinding.descriptorCount       = 1;
    materialLayoutBinding.descriptorType        = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    materialLayoutBinding.stageFlags            = VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBinding.pImmutableSamplers    = nullptr;

    VkDescriptorSetLayoutCreateInfo DSL_Materials_CreateInfo{};
    DSL_Materials_CreateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DSL_Materials_CreateInfo.pNext              = nullptr;
    // DSL_Materials_CreateInfo.flags              = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
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
    TexAmbDiffSpecNrmlMaps_LayoutBindings[2].binding            = 2;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[2].descriptorCount    = 1;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[2].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[2].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[2].pImmutableSamplers = nullptr;
    // Specular binding
    TexAmbDiffSpecNrmlMaps_LayoutBindings[3].binding            = 3;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[3].descriptorCount    = 1;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[3].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[3].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[3].pImmutableSamplers = nullptr;
    // Normal binding
    TexAmbDiffSpecNrmlMaps_LayoutBindings[4].binding            = 4;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[4].descriptorCount    = 1;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[4].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[4].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    TexAmbDiffSpecNrmlMaps_LayoutBindings[4].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo DSL_TexAmbDiffSpecNrmlMaps_CreateInfo{};
    DSL_TexAmbDiffSpecNrmlMaps_CreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DSL_TexAmbDiffSpecNrmlMaps_CreateInfo.pNext         = nullptr;
    // DSL_TexAmbDiffSpecNrmlMaps_CreateInfo.flags         = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    DSL_TexAmbDiffSpecNrmlMaps_CreateInfo.bindingCount  = TexAmbDiffSpecNrmlMaps_LayoutBindings.size();
    DSL_TexAmbDiffSpecNrmlMaps_CreateInfo.pBindings     = TexAmbDiffSpecNrmlMaps_LayoutBindings.data();

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &DSL_TexAmbDiffSpecNrmlMaps_CreateInfo, m_Allocator, &m_DSL_TexAmbDiffSpecNrmlMaps) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanApp - create_descriptors_set_layout] Failed to create TexAmbDiffSpecNrmlMaps descriptor sets layout!");
        return AG_FAILED_TO_CREATE_DESCRIPTOR_SET_LAYOUT;
    }
    // Texture + Ambiant + Diffuse + Specular + Normal Maps Layout Bindings ================

    m_DescriptorSetLayoutDestroyed = false;
    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_descriptor_set_layout] Created all descriptor sets layout!");
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
    std::array<VkDescriptorPoolSize, 8> poolSizes;
    // MVP
    poolSizes[0].type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount    = AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS;
    // EL
    poolSizes[1].type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount    = AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS;
    // Material
    poolSizes[2].type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount    = AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS;
    // Text Sampler
    poolSizes[3].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount    = AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS;
    // Ambt Sampler
    poolSizes[4].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount    = AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS;
    // Diff Sampler
    poolSizes[5].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[5].descriptorCount    = AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS;
    // Spec Sampler
    poolSizes[6].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[6].descriptorCount    = AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS;
    // Normal Sampler
    poolSizes[7].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[7].descriptorCount    = AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS;
    

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
        throw std::runtime_error(   "[Vulkan/VulkanHandler::VulkanApp - create_descriptor_pool] Failed to create descriptor pool!");
        return AG_FAILED_TO_CREATE_DESCRIPTOR_POOL;
    }

    m_DescriptorPoolDestroyed = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_descriptor_pool()
{
    if (!m_DescriptorPoolDestroyed)
    {
        vkDestroyDescriptorPool(m_LogicalDevice, m_DescriptorPool, m_Allocator);
        m_DescriptorPoolDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = descriptor pool = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = descriptor sets = = = = = = = = = = = = = = = = = = = =
// no need to explicitly destroy them, they'll be destroyed automaticly when descriptor pool will be destroyed
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_descriptor_sets()
{
    for (size_t entity_index = 0; entity_index < m_ToRenderEntities.size(); entity_index++)
    {
        // * Descriptor sets allocations
        allocate_descriptor_sets_for_entity(entity_index);

        // * Descriptor Sets Updates
        update_descriptor_sets_for_entity(entity_index);
    }
    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = descriptor sets = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = descriptor sets helpers = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::allocate_descriptor_sets_for_entity(const size_t& entity_index)
{
    m_DS_MVP_EL.resize                  (m_ToRenderEntities.size());
    m_DS_Material.resize                (m_ToRenderEntities.size());
    m_DS_TextAmbtDiffSpecNrmlMaps.resize(m_ToRenderEntities.size());

    // m_DS_MVP_EL
    std::vector<VkDescriptorSetLayout> layouts { m_DSL_MVP_EL };
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext                 = nullptr;
    allocInfo.descriptorPool        = m_DescriptorPool;
    allocInfo.descriptorSetCount    = 1;
    allocInfo.pSetLayouts           = layouts.data();

    if (vkAllocateDescriptorSets(m_LogicalDevice, &allocInfo, &m_DS_MVP_EL[entity_index]) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_descriptor_sets] Failed to allocate MVP EL descriptor sets for entity_index : " + std::to_string(entity_index) + std::string("!"));
        return AG_FAILED_TO_ALLOCATE_DESCRIPTOR_SETS;
    }

    // m_DS_Material
    layouts = std::move(std::vector<VkDescriptorSetLayout>({m_DSL_Material}));
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext                 = nullptr;
    allocInfo.descriptorPool        = m_DescriptorPool;
    allocInfo.descriptorSetCount    = 1;
    allocInfo.pSetLayouts           = layouts.data();


    if (vkAllocateDescriptorSets(m_LogicalDevice, &allocInfo, &m_DS_Material[entity_index]) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_descriptor_sets] Failed to allocate Material descriptor sets for entity_index : " + std::to_string(entity_index) + std::string("!"));
        return AG_FAILED_TO_ALLOCATE_DESCRIPTOR_SETS;
    }

    // m_DS_TextAmbtDiffSpecNrmlMaps
    layouts = std::move(std::vector<VkDescriptorSetLayout>({m_DSL_TexAmbDiffSpecNrmlMaps}));
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext                 = nullptr;
    allocInfo.descriptorPool        = m_DescriptorPool;
    allocInfo.descriptorSetCount    = 1;
    allocInfo.pSetLayouts           = layouts.data();

    if (vkAllocateDescriptorSets(m_LogicalDevice, &allocInfo, &m_DS_TextAmbtDiffSpecNrmlMaps[entity_index]) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_descriptor_sets] Failed to allocate Texture, Ambiant, Diffuse, Specular, Normal maps descriptor sets for entity_index : " + std::to_string(entity_index) + std::string("!"));
        return AG_FAILED_TO_ALLOCATE_DESCRIPTOR_SETS;
    }

    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::update_descriptor_sets_for_entity(const size_t& entity_index)
{
    // MVP_EL_DescriptorWrites
    VkDescriptorBufferInfo MVP_BufferInfo{};
    MVP_BufferInfo.buffer       = m_UBO_MVP_EL.buffer;
    MVP_BufferInfo.offset       = 0;
    MVP_BufferInfo.range        = sizeof(VulkanEntity::UBO::MVP);
    VkDescriptorBufferInfo EL_BufferInfo{};
    EL_BufferInfo.buffer        = m_UBO_MVP_EL.buffer;
    EL_BufferInfo.offset        = calculate_ubo_mvp_el_buffer_size() - sizeof(VulkanEntity::UBO::EnvLight);
    EL_BufferInfo.range         = sizeof(VulkanEntity::UBO::EnvLight);
    VkDescriptorBufferInfo MVP_EL_BufferInfos[] = { MVP_BufferInfo, EL_BufferInfo };

    VkWriteDescriptorSet MVP_EL_DescriptorWrites{};
    MVP_EL_DescriptorWrites.sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    MVP_EL_DescriptorWrites.dstSet              = m_DS_MVP_EL[entity_index];
    MVP_EL_DescriptorWrites.dstBinding          = 0;
    MVP_EL_DescriptorWrites.dstArrayElement     = 0;
    MVP_EL_DescriptorWrites.descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    MVP_EL_DescriptorWrites.descriptorCount     = 2;
    MVP_EL_DescriptorWrites.pBufferInfo         = MVP_EL_BufferInfos;


    // Materials_DescriptorWrites
    VkDescriptorBufferInfo  Material_BufferInfo{};
    Material_BufferInfo.buffer  = m_UBO_Material.buffer;
    Material_BufferInfo.offset  = 0;
    Material_BufferInfo.range   = sizeof(VulkanEntity::UBO::Materials);

    VkWriteDescriptorSet Material_DescriptorWrite{};
    Material_DescriptorWrite.sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Material_DescriptorWrite.dstSet             = m_DS_Material[entity_index];
    Material_DescriptorWrite.dstBinding         = 0;
    Material_DescriptorWrite.dstArrayElement    = 0;
    Material_DescriptorWrite.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    Material_DescriptorWrite.descriptorCount    = 1;
    Material_DescriptorWrite.pBufferInfo        = &Material_BufferInfo;

    // Texture, Ambiant, Diffuse, Specular, Normal maps DescriptorWrites
    std::array<VkDescriptorImageInfo, 5> ImagesInfos{{}};
    // Texture
    ImagesInfos[0].imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImagesInfos[0].imageView    = m_ToRenderEntities[entity_index]->get_entity_data().materials.texture.m_ImageView;
    ImagesInfos[0].sampler      = m_ToRenderEntities[entity_index]->get_entity_data().materials.texture.m_ImageSampler;
    // Ambiant
    ImagesInfos[1].imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImagesInfos[1].imageView    = m_ToRenderEntities[entity_index]->get_entity_data().materials.ambtMap.m_ImageView;
    ImagesInfos[1].sampler      = m_ToRenderEntities[entity_index]->get_entity_data().materials.ambtMap.m_ImageSampler;
    // Diffuse
    ImagesInfos[2].imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImagesInfos[2].imageView    = m_ToRenderEntities[entity_index]->get_entity_data().materials.diffMap.m_ImageView;
    ImagesInfos[2].sampler      = m_ToRenderEntities[entity_index]->get_entity_data().materials.diffMap.m_ImageSampler;
    // Specular
    ImagesInfos[3].imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImagesInfos[3].imageView    = m_ToRenderEntities[entity_index]->get_entity_data().materials.specMap.m_ImageView;
    ImagesInfos[3].sampler      = m_ToRenderEntities[entity_index]->get_entity_data().materials.specMap.m_ImageSampler;
    // Normal
    ImagesInfos[4].imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImagesInfos[4].imageView    = m_ToRenderEntities[entity_index]->get_entity_data().materials.nrmlMap.m_ImageView;
    ImagesInfos[4].sampler      = m_ToRenderEntities[entity_index]->get_entity_data().materials.nrmlMap.m_ImageSampler;

    std::array<VkWriteDescriptorSet, 5> Images_DescriptorWrites{{}};
    // Texture
    Images_DescriptorWrites[0].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Images_DescriptorWrites[0].dstSet           = m_DS_TextAmbtDiffSpecNrmlMaps[entity_index];
    Images_DescriptorWrites[0].dstBinding       = 0;
    Images_DescriptorWrites[0].dstArrayElement  = 0;
    Images_DescriptorWrites[0].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    Images_DescriptorWrites[0].descriptorCount  = 1;
    Images_DescriptorWrites[0].pImageInfo       = &ImagesInfos[0];
    // Ambiant
    Images_DescriptorWrites[1].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Images_DescriptorWrites[1].dstSet           = m_DS_TextAmbtDiffSpecNrmlMaps[entity_index];
    Images_DescriptorWrites[1].dstBinding       = 1;
    Images_DescriptorWrites[1].dstArrayElement  = 0;
    Images_DescriptorWrites[1].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    Images_DescriptorWrites[1].descriptorCount  = 1;
    Images_DescriptorWrites[1].pImageInfo       = &ImagesInfos[1];
    // Diffuse
    Images_DescriptorWrites[2].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Images_DescriptorWrites[2].dstSet           = m_DS_TextAmbtDiffSpecNrmlMaps[entity_index];
    Images_DescriptorWrites[2].dstBinding       = 2;
    Images_DescriptorWrites[2].dstArrayElement  = 0;
    Images_DescriptorWrites[2].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    Images_DescriptorWrites[2].descriptorCount  = 1;
    Images_DescriptorWrites[2].pImageInfo       = &ImagesInfos[2];
    // Specular
    Images_DescriptorWrites[3].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Images_DescriptorWrites[3].dstSet           = m_DS_TextAmbtDiffSpecNrmlMaps[entity_index];
    Images_DescriptorWrites[3].dstBinding       = 3;
    Images_DescriptorWrites[3].dstArrayElement  = 0;
    Images_DescriptorWrites[3].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    Images_DescriptorWrites[3].descriptorCount  = 1;
    Images_DescriptorWrites[3].pImageInfo       = &ImagesInfos[3];
    // Normal
    Images_DescriptorWrites[4].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Images_DescriptorWrites[4].dstSet           = m_DS_TextAmbtDiffSpecNrmlMaps[entity_index];
    Images_DescriptorWrites[4].dstBinding       = 4;
    Images_DescriptorWrites[4].dstArrayElement  = 0;
    Images_DescriptorWrites[4].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    Images_DescriptorWrites[4].descriptorCount  = 1;
    Images_DescriptorWrites[4].pImageInfo       = &ImagesInfos[4];

    std::array<VkWriteDescriptorSet, 7> descriptorWrites;
    descriptorWrites[0] = MVP_EL_DescriptorWrites;
    descriptorWrites[1] = Material_DescriptorWrite;
    descriptorWrites[2] = Images_DescriptorWrites[0];
    descriptorWrites[3] = Images_DescriptorWrites[1];
    descriptorWrites[4] = Images_DescriptorWrites[2];
    descriptorWrites[5] = Images_DescriptorWrites[3];
    descriptorWrites[6] = Images_DescriptorWrites[4];

    vkUpdateDescriptorSets(m_LogicalDevice, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = descriptor sets helpers = = = = = = = = = = = = = = = = = = = =
// ** Descriptors managment (descriptor layout, pool and sets) ======================================================================================


// ** Graphics Pipeline managment ===================================================================================================================
// * = = = = = = = = = = = = = = = = = = = = graphics pipelines = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_graphics_pipeline()
{
    Agos::VulkanHandler::VulkanApp::create_graphics_pipeline_layout();

    VkShaderModule vertShaderModule = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::create_shader_module(
        m_LogicalDevice, std::string(AG_SHADERS_PATH) + "/" + std::string(AG_DEFAULT_VERTEX_SHADER_FOLDER), m_Allocator);
    VkShaderModule fragShaderModule = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::create_shader_module(
        m_LogicalDevice, std::string(AG_SHADERS_PATH) + "/" + std::string(AG_DEFAULT_FRAGMENT_SHADER_FOLDER), m_Allocator);

    Agos::VulkanHandler::VulkanApp::create_graphics_pipeline(vertShaderModule, fragShaderModule);

    vkDestroyShaderModule(m_LogicalDevice, vertShaderModule, m_Allocator);
    vkDestroyShaderModule(m_LogicalDevice, fragShaderModule, m_Allocator);

    m_GraphicsPipelineLayoutDestroyed   = false;
    m_GraphicsPipelineDestroyed         = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_graphics_pipeline()
{
    if (!m_GraphicsPipelineDestroyed)
    {
        vkDestroyPipeline       (m_LogicalDevice, m_GraphicsPipeline        , m_Allocator);
        vkDestroyPipelineLayout (m_LogicalDevice, m_GraphicsPipelineLayout  , m_Allocator);

        m_GraphicsPipelineDestroyed         = true;
        m_GraphicsPipelineLayoutDestroyed   = true;

        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = graphics pipelines = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = graphics pipelines helpers = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_graphics_pipeline_layout()
{
    std::vector<VkDescriptorSetLayout> layouts = { m_DSL_MVP_EL, m_DSL_Material, m_DSL_TexAmbDiffSpecNrmlMaps };
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount           = layouts.size();
    createInfo.pSetLayouts              = layouts.data();
    createInfo.pushConstantRangeCount   = 0;
    createInfo.pPushConstantRanges      = nullptr;

    if ( vkCreatePipelineLayout(m_LogicalDevice, &createInfo, m_Allocator, &m_GraphicsPipelineLayout) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_graphics_pipeline] Failed to create graphics pipeline layout!");
        return AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE_LAYOUT;
    }

    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::create_graphics_pipeline(
    const VkShaderModule& vertex_shader_module,
    const VkShaderModule& fragment_shader_module
)
{
    // shader stages
    VkPipelineShaderStageCreateInfo vertexStageInfo{};
    vertexStageInfo.sType       = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStageInfo.stage       = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageInfo.module      = vertex_shader_module;
    vertexStageInfo.pName       = "main";

    VkPipelineShaderStageCreateInfo fragmentStageInfo{};
    fragmentStageInfo.sType     = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStageInfo.stage     = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageInfo.module    = fragment_shader_module;
    fragmentStageInfo.pName     = "main";
    VkPipelineShaderStageCreateInfo shaderStages[]  = {vertexStageInfo, fragmentStageInfo};

    // vertex input info
    VkVertexInputBindingDescription bindingDescription                      = Agos::VulkanHandler::VulkanEntity::Vertex::get_binding_description();
    std::array<VkVertexInputAttributeDescription, 9> attributeDescriptions  = Agos::VulkanHandler::VulkanEntity::Vertex::get_attribute_description();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                             = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology                          = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable            = VK_FALSE;

    // dynamic states
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount  = 2;
    dynamicState.pDynamicStates     = dynamicStates;

    // viewport and scissor // optional since DYNAMIC STATE
/*
    VkViewport viewport{};
    viewport.x                          = 0.0f;
    viewport.y                          = 0.0f;
    viewport.width                      = static_cast<float>(m_OffscreenExtent.width);
    viewport.height                     = static_cast<float>(m_OffscreenExtent.height);
    viewport.minDepth                   = 0.0f;
    viewport.maxDepth                   = 1.0f;
    VkRect2D scissor{};
    scissor.offset                      = {0, 0};
    scissor.extent                      = m_OffscreenExtent;
*/
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType                 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount         = 1;
    viewportState.scissorCount          = 1;
    // viewportState.pViewports            = &viewport;
    // viewportState.pScissors             = &scissor;

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable         = VK_FALSE;
    rasterizer.rasterizerDiscardEnable  = VK_FALSE;
    rasterizer.polygonMode              = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth                = 1.0f;
    rasterizer.cullMode                 = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace                = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable          = VK_FALSE;
    rasterizer.depthBiasConstantFactor  = 0.0f;     // optional
    rasterizer.depthBiasClamp           = 0.0f;     // optional
    rasterizer.depthBiasSlopeFactor     = 0.0f;     // optional

    // msaa
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = m_MsaaSamples;

    // depth stencil
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable        = VK_TRUE;
    depthStencil.depthWriteEnable       = VK_TRUE;
    depthStencil.depthCompareOp         = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable  = VK_FALSE;
    depthStencil.stencilTestEnable      = VK_FALSE;

    // color blending
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

    // pipelineCreateInfo
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType                    = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount               = 2;
    pipelineCreateInfo.pStages                  = shaderStages;
    pipelineCreateInfo.pVertexInputState        = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState      = &inputAssembly;
    pipelineCreateInfo.pDynamicState            = &dynamicState;
    pipelineCreateInfo.pViewportState           = &viewportState;   // optional
    pipelineCreateInfo.pRasterizationState      = &rasterizer;
    pipelineCreateInfo.pMultisampleState        = &multisampling;
    pipelineCreateInfo.pDepthStencilState       = &depthStencil;
    pipelineCreateInfo.pColorBlendState         = &colorBlending;
    pipelineCreateInfo.layout                   = m_GraphicsPipelineLayout;
    pipelineCreateInfo.renderPass               = m_OffscreenRenderPass;
    pipelineCreateInfo.subpass                  = 0;
    pipelineCreateInfo.basePipelineHandle       = VK_NULL_HANDLE;


    if ( vkCreateGraphicsPipelines(m_LogicalDevice, m_GraphicsPipelineCache, 1, &pipelineCreateInfo, m_Allocator, &m_GraphicsPipeline) != VK_SUCCESS )
    {
        AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanApp - create_graphics_pipeline] Failed to create graphics pipeline!");
        return AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE;
    }
    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_graphics_pipeline] Created graphics pipeline!");

    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = graphics pipelines helpers = = = = = = = = = = = = = = = = = = = =
// ** Graphics Pipeline managment ===================================================================================================================

// ** Vertex, Index, Uniform buffers managment ======================================================================================================
// * = = = = = = = = = = = = = = = = = = = = Vertex Buffers = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_vertex_buffers()
{
    m_EntitiesVertexBuffers.resize(m_ToRenderEntities.size());

    for (size_t entity = 0; entity < m_ToRenderEntities.size(); entity++)
    {
        // VkDeviceSize bufferSize = sizeof(VulkanEntity::Vertex) * sizeof(m_ToRenderEntities[entity]->get_entity_data().vertices.size());
        VkDeviceSize bufferSize = sizeof(m_ToRenderEntities[entity]->get_entity_data().vertices[0]) * m_ToRenderEntities[entity]->get_entity_data().vertices.size();

        VulkanHelpers::AgBuffer stagingBuffer = VulkanHelpers::create_buffer(
            m_PhysicalDevice,
            m_LogicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_Allocator
        );

        void* data;
        vkMapMemory     (m_LogicalDevice, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_ToRenderEntities[entity]->get_entity_data().vertices.data(), (size_t)bufferSize);
        vkUnmapMemory   (m_LogicalDevice, stagingBuffer.bufferMemory);

        m_EntitiesVertexBuffers[entity] = VulkanHelpers::create_buffer(
            m_PhysicalDevice,
            m_LogicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_Allocator
        );

        VulkanHelpers::copy_buffer(
            m_LogicalDevice,
            m_GraphicsQueue,
            m_CommandBuffersPool,
            stagingBuffer.buffer,
            m_EntitiesVertexBuffers[entity].buffer,
            bufferSize,
            m_Allocator
        );

        // vkDestroyBuffer (m_LogicalDevice, stagingBuffer.buffer,         m_Allocator);
        // vkFreeMemory    (m_LogicalDevice, stagingBuffer.bufferMemory,   m_Allocator);
        // * or
        VulkanHelpers::destroy_buffer(
            m_LogicalDevice,
            stagingBuffer,
            m_Allocator
        );
    }

    m_EntitiesVertexBuffersDestroyed = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_vertex_buffers()
{
    if (!m_EntitiesVertexBuffersDestroyed)
    {
        for (size_t entity = 0; entity < m_ToRenderEntities.size(); entity++)
        {
            vkDestroyBuffer (m_LogicalDevice, m_EntitiesVertexBuffers[entity].buffer,          m_Allocator);
            vkFreeMemory    (m_LogicalDevice, m_EntitiesVertexBuffers[entity].bufferMemory,    m_Allocator);
        }
        m_EntitiesVertexBuffersDestroyed = true;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::update_vertex_buffers(/*const uint32_t& image_index*/)
{
    // ! REWORK TO UPDATE Entities VERTICIES WHO HAVE ONLY CHANGED
    for (size_t entity = 0; entity < m_ToRenderEntities.size(); entity++)
    {
        VkDeviceSize bufferSize = sizeof(m_ToRenderEntities[entity]->get_entity_data().vertices[0]) * m_ToRenderEntities[entity]->get_entity_data().vertices.size();

        VulkanHelpers::AgBuffer stagingBuffer = Agos::VulkanHandler::VulkanHelpers::create_buffer(
            m_PhysicalDevice,
            m_LogicalDevice,
            bufferSize,
            // VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            m_Allocator
        );

        void* data;
        vkMapMemory(m_LogicalDevice, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_ToRenderEntities[entity]->get_entity_data().vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_LogicalDevice, stagingBuffer.bufferMemory);

        Agos::VulkanHandler::VulkanHelpers::copy_buffer(
            m_LogicalDevice,
            m_GraphicsQueue,
            m_CommandBuffersPool,
            stagingBuffer.buffer,
            m_EntitiesVertexBuffers[entity].buffer,
            bufferSize,
            m_Allocator
        );

        Agos::VulkanHandler::VulkanHelpers::destroy_buffer(
            m_LogicalDevice,
            stagingBuffer,
            m_Allocator
        );
    }
    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = Vertex Buffers = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = Index Buffers = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_index_buffers()
{
    m_EntitiesIndexBuffer.resize(m_ToRenderEntities.size());

    for (size_t entity = 0; entity < m_ToRenderEntities.size(); entity++)
    {
        // VkDeviceSize bufferSize = sizeof(uint32_t) * m_ToRenderEntities[entity]->get_entity_data().indices.size();
        VkDeviceSize bufferSize = sizeof(m_ToRenderEntities[entity]->get_entity_data().indices[0]) * m_ToRenderEntities[entity]->get_entity_data().indices.size();

        VulkanHelpers::AgBuffer stagingBuffer = VulkanHelpers::create_buffer(
            m_PhysicalDevice,
            m_LogicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_Allocator
        );

        void* data;
        vkMapMemory     (m_LogicalDevice, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_ToRenderEntities[entity]->get_entity_data().indices.data(), (size_t)bufferSize);
        vkUnmapMemory   (m_LogicalDevice, stagingBuffer.bufferMemory);

        m_EntitiesIndexBuffer[entity] = VulkanHelpers::create_buffer(
            m_PhysicalDevice,
            m_LogicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_Allocator
        );

        VulkanHelpers::copy_buffer(
            m_LogicalDevice,
            m_GraphicsQueue,
            m_CommandBuffersPool,
            stagingBuffer.buffer,
            m_EntitiesIndexBuffer[entity].buffer,
            bufferSize,
            m_Allocator
        );

        VulkanHelpers::destroy_buffer(
            m_LogicalDevice,
            stagingBuffer,
            m_Allocator
        );
    }
    m_EntitiesIndexBufferDestroyed = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_index_buffers()
{
    if (!m_EntitiesIndexBufferDestroyed)
    {
        for (size_t entity = 0; entity < m_ToRenderEntities.size(); entity++)
        {
            vkDestroyBuffer (m_LogicalDevice, m_EntitiesIndexBuffer[entity].buffer,        m_Allocator);
            vkFreeMemory    (m_LogicalDevice, m_EntitiesIndexBuffer[entity].bufferMemory,  m_Allocator);
        }
        m_EntitiesIndexBufferDestroyed = true;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::update_index_buffers(/*const uint32_t& image_index*/)
{
    // ! REWORK TO UPDATE Entities VERTICIES WHO HAVE ONLY CHANGED
    for (size_t entity = 0; entity < m_ToRenderEntities.size(); entity++)
    {
        VkDeviceSize bufferSize = sizeof(m_ToRenderEntities[entity]->get_entity_data().indices[0]) * m_ToRenderEntities[entity]->get_entity_data().indices.size();

        VulkanHelpers::AgBuffer stagingBuffer = Agos::VulkanHandler::VulkanHelpers::create_buffer(
            m_PhysicalDevice,
            m_LogicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            m_Allocator
        );

        void* data;
        vkMapMemory(m_LogicalDevice, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_ToRenderEntities[entity]->get_entity_data().indices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_LogicalDevice, stagingBuffer.bufferMemory);

        Agos::VulkanHandler::VulkanHelpers::copy_buffer(
            m_LogicalDevice,
            m_GraphicsQueue,
            m_CommandBuffersPool,
            stagingBuffer.buffer,
            m_EntitiesVertexBuffers[entity].buffer,
            bufferSize,
            m_Allocator
        );

        Agos::VulkanHandler::VulkanHelpers::destroy_buffer(
            m_LogicalDevice,
            stagingBuffer,
            m_Allocator
        );
    }
    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = Index Buffers = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = Uniform Buffers = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_uniform_buffers()
{
    VkDeviceSize MVP_EL_BufferSize      = calculate_ubo_mvp_el_buffer_size();
    VkDeviceSize Material_BufferSize    = sizeof(Agos::VulkanHandler::VulkanEntity::UBO::Materials);

    // ! WIP, Rework for only one allocation ============================
    m_UBO_MVP_EL = Agos::VulkanHandler::VulkanHelpers::create_buffer(
        m_PhysicalDevice,
        m_LogicalDevice,
        MVP_EL_BufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_Allocator
    );

    m_UBO_Material = Agos::VulkanHandler::VulkanHelpers::create_buffer(
        m_PhysicalDevice,
        m_LogicalDevice,
        Material_BufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_Allocator
    );

    m_UniformBuffersDestroyed = false;
    AG_CORE_INFO("[Vulkan/VulkanHandler::VulkanApp - create_uniform_buffers] Created uniform buffers!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_uniform_buffers()
{
    if (!m_UniformBuffersDestroyed)
    {
        vkDestroyBuffer (m_LogicalDevice, m_UBO_MVP_EL.buffer,         m_Allocator);
        vkFreeMemory    (m_LogicalDevice, m_UBO_MVP_EL.bufferMemory,   m_Allocator);
        vkDestroyBuffer (m_LogicalDevice, m_UBO_Material.buffer,       m_Allocator);
        vkFreeMemory    (m_LogicalDevice, m_UBO_Material.bufferMemory, m_Allocator);

        m_UniformBuffersDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::update_uniform_buffers()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


    Agos::VulkanHandler::VulkanEntity::UBO::MVP       ubo_MVP;
    // Agos::VulkanHandler::VulkanEntity::UBO::Materials ubo_Materials;
    Agos::VulkanHandler::VulkanEntity::UBO::EnvLight  ubo_EnvLight;

    // ubo_MVP.model   = glm::mat4(1.0f);
    ubo_MVP.model   = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo_MVP.view    = glm::lookAt(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo_MVP.proj    = glm::perspective(glm::radians(80.0f), m_OffscreenViewportSize.x / m_OffscreenViewportSize.y, 0.0625f, 500.0f);
    ubo_MVP.proj[1][1] *= -1;

    // ubo_EnvLight.lightColor = glm::vec3(0.25f * time, 1.0f - 0.25f*time, 1.0f - 0.25f*time);
    ubo_EnvLight.lightColor = glm::vec3(1.0f);
    ubo_EnvLight.lightPos   = glm::vec3(100.0f, 100.0f, 100.0f);

    for (size_t entity = 0; entity < m_ToRenderEntities.size(); entity++)
    {
        // MVP
        VkDeviceSize uboSize = sizeof(Agos::VulkanHandler::VulkanEntity::UBO::MVP);
        void* data;
        vkMapMemory(m_LogicalDevice, m_UBO_MVP_EL.bufferMemory, 0, uboSize, 0, &data);
        memcpy(data, &ubo_MVP, (size_t)uboSize);
        vkUnmapMemory(m_LogicalDevice, m_UBO_MVP_EL.bufferMemory);

        // EnvLight
        uboSize = sizeof(Agos::VulkanHandler::VulkanEntity::UBO::EnvLight);
        data = nullptr;
        vkMapMemory(m_LogicalDevice, m_UBO_MVP_EL.bufferMemory, calculate_ubo_mvp_el_buffer_size() - uboSize, uboSize, 0, &data);
        memcpy(data, &ubo_EnvLight, (size_t)uboSize);
        vkUnmapMemory(m_LogicalDevice, m_UBO_MVP_EL.bufferMemory);
    }
    
    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = Uniform Buffers = = = = = = = = = = = = = = = = = = = =
// * = = = = = = = = = = = = = = = = = = = = UBOs helpers = = = = = = = = = = = = = = = = = = = =
// minUniformBufferOffsetAlighment limit constraints must be met
VkDeviceSize Agos::VulkanHandler::VulkanApp::calculate_ubo_mvp_el_buffer_size()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);
    
    std::ldiv_t res = std::ldiv((VkDeviceSize)sizeof(Agos::VulkanHandler::VulkanEntity::UBO::MVP), physicalDeviceProperties.limits.minUniformBufferOffsetAlignment);
    if (res.rem == 0)
    {
        return (VkDeviceSize)sizeof(Agos::VulkanHandler::VulkanEntity::UBO::MVP) + sizeof(Agos::VulkanHandler::VulkanEntity::UBO::EnvLight);
    }
    else
    {
        return (VkDeviceSize)(res.quot + 1) * physicalDeviceProperties.limits.minUniformBufferOffsetAlignment + sizeof(Agos::VulkanHandler::VulkanEntity::UBO::EnvLight);
    }
}
// * = = = = = = = = = = = = = = = = = = = = UBOs helpers = = = = = = = = = = = = = = = = = = = =
// ** Vertex, Index, Uniform buffers managment ======================================================================================================

// ** Command Buffers and Command Pool managment ====================================================================================================
// * = = = = = = = = = = = = = = = = = = = = command buffers = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::record_command_buffers(const uint32_t& image_index)
{
    m_CommandBuffers.resize(m_SwapchainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext                 = nullptr;
    allocInfo.commandPool           = m_CommandBuffersPool;
    allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount    = m_CommandBuffers.size();

    if ( vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS )
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_command_buffers] Failed to allocate command buffers!");
        return AG_FAILED_TO_ALLOCATE_COMMAND_BUFFERS;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext             = nullptr;
    beginInfo.pInheritanceInfo  = nullptr;
    beginInfo.flags             = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if ( vkBeginCommandBuffer(m_CommandBuffers[image_index], &beginInfo) != VK_SUCCESS )
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_command_buffers] Failed to begin command buffers (image_index : " + std::to_string(image_index) + "!");
        return AG_FAILED_TO_RECORD_COMMAND_BUFFERS;
    }

    // VulkanApp's entities
    if (m_SceneState->shall_draw_viewport)
    {
        // render pass begin info
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType                = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext                = nullptr;
        renderPassInfo.renderPass           = m_OffscreenRenderPass;
        renderPassInfo.framebuffer          = m_OffscreenFramebuffer;
        renderPassInfo.renderArea.offset    = {0, 0};
        renderPassInfo.renderArea.extent    = m_OffscreenExtent;

        std::array<VkClearValue, 2> clearValues{{}};
        clearValues[0].color        = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount  = clearValues.size();
        renderPassInfo.pClearValues     = clearValues.data();

        // viewport and scissor
        VkViewport viewPort{};
        viewPort.width      = m_OffscreenExtent.width;
        viewPort.height     = m_OffscreenExtent.height;
        viewPort.minDepth   = 0.0f;
        viewPort.maxDepth   = 1.0f;
        VkRect2D scissor{};
        scissor.extent  = m_OffscreenExtent;
        scissor.offset  = {0, 0};

        vkCmdBeginRenderPass(m_CommandBuffers[image_index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdSetViewport    (m_CommandBuffers[image_index], 0, 1, &viewPort);
        vkCmdSetScissor     (m_CommandBuffers[image_index], 0, 1, &scissor);
        vkCmdBindPipeline   (m_CommandBuffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

        for (size_t i = 0; i < m_ToRenderEntities.size(); i++)
        {
            VkBuffer vertexBuffers[]    = { m_EntitiesVertexBuffers[i].buffer };
            VkBuffer  indexBuffer       = m_EntitiesIndexBuffer[i].buffer;
            VkDeviceSize offsets[]      = { 0 };

            vkCmdBindVertexBuffers  (m_CommandBuffers[image_index], 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer    (m_CommandBuffers[image_index], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets (
                m_CommandBuffers[image_index],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_GraphicsPipelineLayout,
                0,
                1,
                &m_DS_MVP_EL[i],
                0,
                nullptr
            );

            vkCmdBindDescriptorSets (
                m_CommandBuffers[image_index],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_GraphicsPipelineLayout,
                1,
                1,
                &m_DS_Material[i],
                0,
                nullptr
            );

            vkCmdBindDescriptorSets (
                m_CommandBuffers[image_index],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_GraphicsPipelineLayout,
                2,
                1,
                &m_DS_TextAmbtDiffSpecNrmlMaps[i],
                0,
                nullptr
            );

            vkCmdDrawIndexed(m_CommandBuffers[image_index], static_cast<uint32_t>( m_ToRenderEntities[i]->get_entity_data().indices.size() ), 1, 0, 0, 0);
        }
        vkCmdEndRenderPass(m_CommandBuffers[image_index]);
    }

    // ImGui
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType                = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext                = nullptr;
        renderPassInfo.renderPass           = m_RenderPass;
        renderPassInfo.framebuffer          = m_SwapchainFramebuffers[image_index];
        renderPassInfo.renderArea.offset    = {0, 0};
        renderPassInfo.renderArea.extent    = m_SwapchainExtent;

        VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

        renderPassInfo.clearValueCount  = 1;
        renderPassInfo.pClearValues     = &clearValue;

        vkCmdBeginRenderPass        (m_CommandBuffers[image_index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_ImGuiInterface->draw_frame(m_CommandBuffers[image_index]);
        vkCmdEndRenderPass          (m_CommandBuffers[image_index]);
    }

    if ( vkEndCommandBuffer(m_CommandBuffers[image_index]) != VK_SUCCESS )
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_command_buffers] Failed to record command buffers (image_index[" + std::to_string(image_index) + "]!");
        return AG_FAILED_TO_RECORD_COMMAND_BUFFERS;
    }

    m_CommandBuffersDestroyed = false;  // ~ is gonna repeat - WIP
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_command_buffers()
{
    if (!m_CommandBuffersDestroyed)
    {
        vkFreeCommandBuffers(m_LogicalDevice, m_CommandBuffersPool, m_CommandBuffers.size(), m_CommandBuffers.data());
        m_CommandBuffersDestroyed = true;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = command buffers = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = command buffers pool = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_command_buffers_pool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext              = nullptr;
    poolInfo.flags              = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // ? potentiels bugs?
    poolInfo.queueFamilyIndex   = VulkanHandler::VulkanBase::find_queue_families_indices(m_PhysicalDevice, m_WindowSurface).graphics_family.value();

    if ( vkCreateCommandPool(m_LogicalDevice, &poolInfo, m_Allocator, &m_CommandBuffersPool) != VK_SUCCESS )
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_command_buffers_pool] Failed to create command buffers pool!");
        return AG_FAILED_TO_CREATE_COMMAND_BUFFERS_POOL;
    }

    m_CommandBuffersPoolDestroyed = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_command_buffers_pool()
{
    if (!m_CommandBuffersPoolDestroyed)
    {
        vkDestroyCommandPool(m_LogicalDevice, m_CommandBuffersPool, m_Allocator);
        m_CommandBuffersPoolDestroyed = true;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = command buffers pool = = = = = = = = = = = = = = = = = = = =
// ** Command Buffers and Command Pool managment ====================================================================================================

// ** Sync objects - Semaphores and Fences - managment ==============================================================================================
// * = = = = = = = = = = = = = = = = = = = = semaphores = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_sync_objects()
{
    create_semaphore_objects();
    create_fence_objects();
    m_SyncObjectsDestroyed = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::create_semaphore_objects()
{
    m_ImageAvailableSemaphores.resize(AG_VULKAN_MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(AG_VULKAN_MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType        = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext        = nullptr;

    for (size_t i = 0; i < m_ImageAvailableSemaphores.size(); i++)
    {
        if ( vkCreateSemaphore(m_LogicalDevice, &createInfo, m_Allocator, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS )
        {
            AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_semaphore_objects] Failed to create m_ImageAvailableSemaphores[" + std::to_string(i) + std::string("] object!"));
            return AG_FAILED_TO_CREATE_SEMAPHORES_OBJECTS;
        }
    }

    for (size_t i = 0; i < m_RenderFinishedSemaphores.size(); i++)
    {
        if ( vkCreateSemaphore(m_LogicalDevice, &createInfo, m_Allocator, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS )
        {
            AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_semaphore_objects] Failed to create m_RenderFinishedSemaphores[" + std::to_string(i) + std::string("] object!"));
            return AG_FAILED_TO_CREATE_SEMAPHORES_OBJECTS;
        }
    }

    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::create_fence_objects()
{
    m_InFlightFences.resize(AG_VULKAN_MAX_FRAMES_IN_FLIGHT);
    m_ImagesInFlight.resize(m_SwapchainImages.size(), VK_NULL_HANDLE);

    VkFenceCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext    = nullptr;
    createInfo.flags    = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < m_InFlightFences.size(); i++)
    {
        if ( vkCreateFence(m_LogicalDevice, &createInfo, m_Allocator, &m_InFlightFences[i]) != VK_SUCCESS )
        {
            AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_fence_objects] Failed to create m_InFlightFences[" + std::to_string(i) + std::string("] object!"));
            return AG_FAILED_TO_CREATE_FENCE_OBJECTS;
        }
    }

    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_sync_objects()
{
    if (!m_SyncObjectsDestroyed)
    {
        for (const VkSemaphore& it : m_ImageAvailableSemaphores)
        {
            vkDestroySemaphore(m_LogicalDevice, it, m_Allocator);
        }
        for (const VkSemaphore& it : m_RenderFinishedSemaphores)
        {
            vkDestroySemaphore(m_LogicalDevice, it, m_Allocator);
        }
        for (const VkFence& it : m_InFlightFences)
        {
            vkDestroyFence(m_LogicalDevice, it, m_Allocator);
        }
        m_SyncObjectsDestroyed = true;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = semaphores = = = = = = = = = = = = = = = = = = = =
// ** Sync objects - Semaphores and Fences - managment ==============================================================================================

// ** Entities misc managment =======================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::load_entities_textures()
{
    for (size_t i = 0; i < m_ToRenderEntities.size(); i++)
    {
        // thing here is : each entity contains wrappers around a texture file and a sampler, and thus provide a more convenient way of dealing textures' allocations

        // ENTITY'S TEXTURE
        if (m_ToRenderEntities[i]->get_entity_data().materials.has_texture)
        {
            m_ToRenderEntities[i]->get_entity_data().materials.texture.create_sampler(
                m_ToRenderEntities[i]->get_entity_data().materials.texture_path,
                m_PhysicalDevice,
                m_LogicalDevice,
                m_GraphicsQueue,
                m_CommandBuffersPool,    // is fine using this pool
                m_Allocator
            );
        }
        else
        {
            m_ToRenderEntities[i]->get_entity_data().materials.texture.create_sampler(
                AG_SOURCE_PATH + std::string(AG_DEFAULT_TEXTURE),
                m_PhysicalDevice,
                m_LogicalDevice,
                m_GraphicsQueue,
                m_CommandBuffersPool,
                m_Allocator
            );
        }

        // ENTITY'S AMBIANT MAP
        if (m_ToRenderEntities[i]->get_entity_data().materials.has_ambiant)
        {
            m_ToRenderEntities[i]->get_entity_data().materials.ambtMap.create_sampler(
                m_ToRenderEntities[i]->get_entity_data().materials.ambiant_path,
                m_PhysicalDevice,
                m_LogicalDevice,
                m_GraphicsQueue,
                m_CommandBuffersPool,    // is fine using this pool
                m_Allocator
            );
        }
        else
        {
            m_ToRenderEntities[i]->get_entity_data().materials.ambtMap.create_sampler(
                AG_SOURCE_PATH + std::string(AG_DEFAULT_TEXTURE),
                m_PhysicalDevice,
                m_LogicalDevice,
                m_GraphicsQueue,
                m_CommandBuffersPool,
                m_Allocator
            );
        }

        // ENTITY'S DIFFUSE MAP
        if (m_ToRenderEntities[i]->get_entity_data().materials.has_diffuse)
        {
            m_ToRenderEntities[i]->get_entity_data().materials.diffMap.create_sampler(
                m_ToRenderEntities[i]->get_entity_data().materials.diffuse_path,
                m_PhysicalDevice,
                m_LogicalDevice,
                m_GraphicsQueue,
                m_CommandBuffersPool,    // is fine using this pool
                m_Allocator
            );
        }
        else
        {
            m_ToRenderEntities[i]->get_entity_data().materials.diffMap.create_sampler(
                AG_SOURCE_PATH + std::string(AG_DEFAULT_TEXTURE),
                m_PhysicalDevice,
                m_LogicalDevice,
                m_GraphicsQueue,
                m_CommandBuffersPool,
                m_Allocator
            );
        }

        // ENTITY'S SPECULAR MAP
        if (m_ToRenderEntities[i]->get_entity_data().materials.has_specular)
        {
            m_ToRenderEntities[i]->get_entity_data().materials.specMap.create_sampler(
                m_ToRenderEntities[i]->get_entity_data().materials.specular_path,
                m_PhysicalDevice,
                m_LogicalDevice,
                m_GraphicsQueue,
                m_CommandBuffersPool,    // is fine using this pool
                m_Allocator
            );
        }
        else
        {
            m_ToRenderEntities[i]->get_entity_data().materials.specMap.create_sampler(
                AG_SOURCE_PATH + std::string(AG_DEFAULT_TEXTURE),
                m_PhysicalDevice,
                m_LogicalDevice,
                m_GraphicsQueue,
                m_CommandBuffersPool,
                m_Allocator
            );
        }

        // ENTITY'S NORMAL MAP
        if (m_ToRenderEntities[i]->get_entity_data().materials.has_normal)
        {
            m_ToRenderEntities[i]->get_entity_data().materials.nrmlMap.create_sampler(
                m_ToRenderEntities[i]->get_entity_data().materials.normal_path,
                m_PhysicalDevice,
                m_LogicalDevice,
                m_GraphicsQueue,
                m_CommandBuffersPool,    // is fine using this pool
                m_Allocator
            );
        }
        else
        {
            m_ToRenderEntities[i]->get_entity_data().materials.nrmlMap.create_sampler(
                AG_SOURCE_PATH + std::string(AG_DEFAULT_TEXTURE),
                m_PhysicalDevice,
                m_LogicalDevice,
                m_GraphicsQueue,
                m_CommandBuffersPool,
                m_Allocator
            );
        }
    }
    m_EntitiesTexturesUnloaded = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::unload_entities_textures()
{
    if (!m_EntitiesTexturesUnloaded)
    {
        for (size_t i = 0; i < m_ToRenderEntities.size(); i++)
        {
            m_ToRenderEntities[i]->get_entity_data().materials.texture.terminate_sampler();
            m_ToRenderEntities[i]->get_entity_data().materials.ambtMap.terminate_sampler();
            m_ToRenderEntities[i]->get_entity_data().materials.diffMap.terminate_sampler();
            m_ToRenderEntities[i]->get_entity_data().materials.specMap.terminate_sampler();
            m_ToRenderEntities[i]->get_entity_data().materials.nrmlMap.terminate_sampler();
        }
        m_EntitiesTexturesUnloaded = true;
        return AG_SUCCESS;
    }
    return AG_ENTITIES_TEXTURES_ALREADY_UNLOADED;
}
// ** Entities misc managment =======================================================================================================================

// ** Offscreen render target managment =============================================================================================================
// * = = = = = = = = = = = = = = = = = = = = offscreen render target = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_offscreen_render_target()
{
    // m_OffscreenExtent.width = AG_DEFAULT_VIEWPORT_WIDTH;
    // m_OffscreenExtent.height = AG_DEFAULT_VIEWPORT_HEIGHT;
    m_OffscreenExtent.width     = AG_MAX_VIEWPORT_WIDTH;
    m_OffscreenExtent.height    = AG_MAX_VIEWPORT_HEIGHT;
    m_OffscreenFormat           = VK_FORMAT_R8G8B8A8_SRGB;

    create_offscreen_image_res      ();
    create_depth_res                ();
    create_color_res                ();
    create_offscreen_render_pass    ();
    create_offscreen_framebuffer    ();
    create_offscreen_sampler        ();
    create_graphics_pipeline        ();
    m_OffscreenRenderTargetDestroyed = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_offscreen_render_target()
{
    if (!m_OffscreenRenderTargetDestroyed)
    {
        unload_entities_textures        ();
        destroy_offscreen_framebuffer   ();
        destroy_graphics_pipeline       ();
        destroy_offscreen_render_pass   ();
        destroy_offscreen_sampler       ();
        destroy_offscreen_image_res     ();
        destroy_depth_res               ();
        destroy_color_res               ();
        m_OffscreenRenderTargetDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = offscreen render target = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = offscreen image ressources = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_offscreen_image_res()
{
    // m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    Agos::VulkanHandler::VulkanHelpers::AgImage offscreenImage = Agos::VulkanHandler::VulkanHelpers::create_image(
        m_PhysicalDevice,
        m_LogicalDevice,
        m_OffscreenExtent.width,
        m_OffscreenExtent.height,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        m_OffscreenFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_Allocator
    );
    m_OffscreenImage        = offscreenImage.image;
    m_OffscreenImageMemory  = offscreenImage.imageMemory;

    m_OffscreenImageView    = Agos::VulkanHandler::VulkanHelpers::create_image_view(
        m_LogicalDevice,
        m_OffscreenImage,
        m_OffscreenFormat,
        VK_IMAGE_ASPECT_COLOR_BIT,
        1,
        m_Allocator
    );

    m_OffscreenImageResDestroyed = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_offscreen_image_res()
{
    if (!m_OffscreenImageResDestroyed)
    {
        vkDestroyImageView  (m_LogicalDevice, m_OffscreenImageView  , m_Allocator);
        vkDestroyImage      (m_LogicalDevice, m_OffscreenImage      , m_Allocator);
        vkFreeMemory        (m_LogicalDevice, m_OffscreenImageMemory, m_Allocator);

        m_OffscreenImageResDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = offscreen image ressources = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = offscreen render pass = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_offscreen_render_pass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format                  = m_OffscreenFormat;
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
    colorAttachmentResolve.format           = m_OffscreenFormat;
    colorAttachmentResolve.samples          = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // colorAttachmentResolve.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentResolve.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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

    std::array<VkSubpassDependency, 1> dependencies({});
    dependencies[0].srcSubpass              = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass              = 0;
    dependencies[0].srcStageMask            = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask           = 0;
    dependencies[0].dstStageMask            = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].dstAccessMask           = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

/*
    std::array<VkSubpassDependency, 2> dependencies({});
    dependencies[0].srcSubpass              = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass              = 0;
    dependencies[0].srcStageMask            = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask            = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask           = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask           = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags         = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass              = 0;
    dependencies[1].dstSubpass              = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask            = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask            = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask           = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask           = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags         = VK_DEPENDENCY_BY_REGION_BIT;
*/

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount        = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments           = attachments.data();
    render_pass_info.subpassCount           = 1;
    render_pass_info.pSubpasses             = &subpass;
    render_pass_info.dependencyCount        = dependencies.size();
    render_pass_info.pDependencies          = dependencies.data();

    if (vkCreateRenderPass(m_LogicalDevice, &render_pass_info, m_Allocator, &m_OffscreenRenderPass) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL(           "[Vulkan/Agos::VulkanHandler::VulkanApp - create_offscreen_render_pass] Failed to create offscreen render pass!");
        throw std::runtime_error(   "[Vulkan/Agos::VulkanHandler::VulkanApp - create_offscreen_render_pass] Failed to create offscreen render pass!");
        return AG_FAILED_TO_CREATE_RENDER_PASS;
    }

    m_OffscreenRenderPassDestroyed = false;
    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanApp - create_offscreen_render_pass] Created offscreen render pass!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_offscreen_render_pass()
{
    if (!m_OffscreenRenderPassDestroyed)
    {
        vkDestroyRenderPass(m_LogicalDevice, m_OffscreenRenderPass, m_Allocator);
        m_OffscreenRenderPassDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = offscreen render pass = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = offscreen framebuffer = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_offscreen_framebuffer()
{
    std::array<VkImageView, 3> attachments = {
        m_ColorImageView,
        m_DepthImageView,
        m_OffscreenImageView
    };

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass       = m_OffscreenRenderPass;
    createInfo.attachmentCount  = static_cast<uint32_t>(attachments.size());
    createInfo.pAttachments     = attachments.data();
    createInfo.width            = m_OffscreenExtent.width;
    createInfo.height           = m_OffscreenExtent.height;
    createInfo.layers           = 1;

    if (vkCreateFramebuffer(m_LogicalDevice, &createInfo, m_Allocator, &m_OffscreenFramebuffer) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_offscreen_framebuffer] Failed to create offscreen framebuffer!");
        return AG_FAILED_TO_CREATE_FRAMEBUFFERS;
    }
    m_OffscreenFramebufferDestoyed = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_offscreen_framebuffer()
{
    if (!m_OffscreenFramebufferDestoyed)
    {
        vkDestroyFramebuffer(m_LogicalDevice, m_OffscreenFramebuffer, m_Allocator);
        m_OffscreenFramebufferDestoyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = offscreen framebuffer = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = offscreen sampler = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_offscreen_sampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext                   = nullptr;
    samplerInfo.magFilter               = VK_FILTER_LINEAR;
    samplerInfo.minFilter               = VK_FILTER_LINEAR;
    samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable        = VK_TRUE;
    samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 1.0f;
    samplerInfo.mipLodBias              = 0.0f;

    if (vkCreateSampler(m_LogicalDevice, &samplerInfo, m_Allocator, &m_OffscreenSampler) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/VulkanHandler::VulkanApp - create_offscreen_sampler] Failed to create offscreen sampler!");
        return AG_FAILED_TO_CREATE_TEXTURE_SAMPLER;
    }
    m_OffscreenSamplerDestroyed = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::destroy_offscreen_sampler()
{
    if (!m_OffscreenSamplerDestroyed)
    {
        vkDestroySampler(m_LogicalDevice, m_OffscreenSampler,m_Allocator);
        m_OffscreenSamplerDestroyed = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = offscreen sampler = = = = = = = = = = = = = = = = = = = =
// ** Offscreen render target managment =============================================================================================================

// ** ImGui managment ===============================================================================================================================
Agos::AgResult Agos::VulkanHandler::VulkanApp::create_imgui_render_target()
{
    create_render_pass              ();
    create_swapchain_framebuffers   ();
    return AG_SUCCESS;
}

Agos::AgResult Agos::VulkanHandler::VulkanApp::draw_imgui_objects()
{
    m_ImGuiInterface->new_frame();


    imgui_draw_main_window();
    imgui_draw_viewport();

    m_ImGuiInterface->end_frame();
    return AG_SUCCESS;
}

void Agos::VulkanHandler::VulkanApp::imgui_draw_main_window()
{
    ImGui::DockSpaceOverViewport();
    ImGui::ShowDemoWindow();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Viewport"))
        {
            ImGui::MenuItem("See Viewport", NULL, &m_SceneState->shall_draw_viewport);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("New..."))
            {
                // ImGui::MenuItem("Scene", "CTRL+N", &create_new_scene);
                // ImGui::MenuItem("Entity", "CTRL+E", &create_new_entity);
                ImGui::EndMenu();
            }

            // ImGui::MenuItem("Save", NULL, &save_current_scene);
            // ImGui::MenuItem("Save as...", NULL, &save_current_scene_as);

            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit"))
        {
            // ImGui::MenuItem("Undo", "CTRL+Z", &triggered_undo, can_undo);
            // ImGui::MenuItem("Redo", "CTRL+Y", &triggered_redo, can_redo);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Agos::VulkanHandler::VulkanApp::imgui_draw_viewport()
{
    if (ImGui::Begin("Viewport")) 
    {
        // m_OffscreenImGuiID = ImGui_ImplVulkan_AddTexture(m_OffscreenSampler, m_OffscreenImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_OffscreenViewportSize = ImGui::GetWindowSize();
        if (m_SceneState->shall_draw_viewport)
        {
            ImGui::Image(m_OffscreenImGuiID, m_OffscreenViewportSize);
        }
        else
        {
            ImGui::GetWindowDrawList()->AddRect(ImVec2(0.0f, 0.0f), m_OffscreenViewportSize, (ImU32)( (255 << 24) | (128 << 16) | (128 << 8) | (128) ));
        }
        
        if (ImGui::IsMouseHoveringRect(ImVec2(0.0f, 0.0f), m_OffscreenViewportSize))
        {
            // do calculation stuff
        }
        ImGui::End();
    }
}
// ** ImGui managment ===============================================================================================================================

Agos::AgResult Agos::VulkanHandler::VulkanApp::process_events(
    const Agos::GLFWHandler::GLFWEvent::Event& event
)
{
    switch (event.type)
    {
        case Agos::GLFWHandler::GLFWEvent::EventType::framebufferResizeCallback:
        {
            // this->m_FramebufferResizedFlag = true;
            this->recreate_swapchain();
            break;
        }
        case Agos::GLFWHandler::GLFWEvent::EventType::mouseButtonCallback:
        {
            // clicky stuff goes brrrrrrrrrr
            break;
        }
        case Agos::GLFWHandler::GLFWEvent::EventType::cursorPosCallback:
        {
            double x, y;
            glfwGetCursorPos(m_GLFWInterfaceRef->get_window(), &x, &y);
            break;
        }
        case Agos::GLFWHandler::GLFWEvent::EventType::keyboardCallback:
        {
            break;
        }
        case Agos::GLFWHandler::GLFWEvent::EventType::undefined:
        {
            AG_CORE_WARN("[GLFW/AgGLFWHandlerInstance - process_event] Undefined event triggered!");
            break;
        }
        case Agos::GLFWHandler::GLFWEvent::EventType::invalid:
        {
            AG_CORE_ERROR("[Event Process /VulkanHandler::VulkanApp - process_event] Invalid event triggered!");
            break;
        }
    }
    return AG_SUCCESS;
}

// * = = = = = = = = = = = = = = = = = = = = <...> helpers = = = = = = = = = = = = = = = = = = = =
// * = = = = = = = = = = = = = = = = = = = = <...> helpers = = = = = = = = = = = = = = = = = = = =
