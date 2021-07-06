#include "Agos/src/renderer/renderer.h"

#include "Agos/src/logger/logger.h"

Agos::AgVulkanHandlerRenderer::AgVulkanHandlerRenderer(const std::shared_ptr<dexode::EventBus>& event_bus)
    : m_EventBus(event_bus)
{
    m_GLFWEventsHandler         = std::make_shared<AgGLFWHandlerEvents>(m_EventBus);
    m_GLFWInstance              = std::make_shared<AgGLFWHandlerInstance>(m_EventBus, this);
    m_VulkanInstance            = std::make_shared<AgVulkanHandlerInstance>(m_EventBus);
    m_VulkanDebugLayersManager  = std::make_shared<AgVulkanHandlerDebugLayersManager>(m_VulkanInstance);

    m_VulkanPhysicalDevice      = std::make_shared<AgVulkanHandlerPhysicalDevice>();
    m_VulkanLogicalDevice       = std::make_shared<AgVulkanHandlerLogicalDevice>();
    m_VulkanSwapChain           = std::make_shared<AgVulkanHandlerSwapChain>();
    m_VulkanRenderPass          = std::make_shared<AgVulkanHandlerRenderPass>();

    m_VulkanDescriptorManager            = std::make_shared<AgVulkanHandlerDescriptorManager>();
    m_VulkanGraphicsPipelineManager      = std::make_shared<AgVulkanHandlerGraphicsPipelineManager>();
    m_VulkanGraphicsCommandPoolManager   = std::make_shared<AgVulkanHandlerCommandPoolManager>();
    m_VulkanColorDepthRessourcesManager  = std::make_shared<AgVulkanHandlerColorDepthRessourcesManager>();
    m_VulkanSwapChainFrameBuffersManager = std::make_shared<AgVulkanHandlerFramebuffers>();
    m_VulkanTextureImageManager          = std::make_shared<AgVulkanHandlerTextureManager>();

    m_Model               = std::make_shared<std::pair<AgModelLoader, AgVertexIndexHolder>>();
    m_VertexIndexUniformBuffers     = std::make_shared<AgVulkanHandlerBufferManager>();
    m_VulkanPresenter               = std::make_shared<AgVulkanHandlerPresenter>();
}

Agos::AgVulkanHandlerRenderer::~AgVulkanHandlerRenderer()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerRenderer::init_vulkan()
{
    AG_CORE_WARN("Initializing GLFW instance...");
    m_GLFWInstance->init(m_GLFWEventsHandler);

    AG_CORE_WARN("Initializing vulkan instance...");
    m_VulkanInstance->init(m_VulkanDebugLayersManager);

    AG_CORE_WARN("Setting up debug layers...");
    m_VulkanDebugLayersManager->vulkan_setup_debug_messenger();

    AG_CORE_WARN("Picking vulkan compatible GPU...");
    m_GLFWInstance->setup_vulkan_surface(m_VulkanInstance);
    m_VulkanPhysicalDevice->pick_physical_device(m_VulkanInstance, m_GLFWInstance);

    AG_CORE_WARN("Creating vulkan logical device...");
    m_VulkanLogicalDevice->create_logical_device(
        m_VulkanPhysicalDevice,
        m_GLFWInstance,
        m_VulkanDebugLayersManager
    );

    AG_CORE_WARN("Creating swap chain...");
    m_VulkanSwapChain->create_swap_chain(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_GLFWInstance
    );
    AG_CORE_WARN("Creating swap chain image views...");
    m_VulkanSwapChain->create_image_views(
        m_VulkanLogicalDevice
    );
    AG_CORE_WARN("Creating render pass...");
    m_VulkanRenderPass->create_render_pass(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    AG_CORE_WARN("Creating descriptors sets layouts...");
    m_VulkanDescriptorManager->create_descriptor_set_layout(
        m_VulkanLogicalDevice
    );
    AG_CORE_WARN("Creating graphics pipeline...");
    m_VulkanGraphicsPipelineManager->create_graphics_pipeline(
        std::string(AG_SHADERS_PATH) + '/',
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanDescriptorManager
    );
    AG_CORE_WARN("Creating graphics command pools...");
    m_VulkanGraphicsCommandPoolManager->create_command_pool(
        m_GLFWInstance,
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    AG_CORE_WARN("Creating color and depths ressources...");
    m_VulkanColorDepthRessourcesManager->create_color_ressources(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    m_VulkanColorDepthRessourcesManager->create_depth_ressources(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    AG_CORE_WARN("Creating swap chain frame buffers...");
    m_VulkanSwapChainFrameBuffersManager->create_framebuffers(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanColorDepthRessourcesManager
    );
    AG_CORE_WARN("Loading texture : " + std::string(AG_MODELS_PATH) + std::string("/viking_room/viking_room.png..."));
    m_VulkanTextureImageManager->create_texture_image(
        std::string(AG_MODELS_PATH) + std::string("/viking_room/viking_room.png"),
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanColorDepthRessourcesManager,
        m_VulkanGraphicsCommandPoolManager,
        m_VertexIndexUniformBuffers
    );
    m_VulkanTextureImageManager->create_texture_image_view(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    m_VulkanTextureImageManager->create_texture_sampler(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice
    );
    AG_CORE_WARN("Loading model : " + std::string(AG_MODELS_PATH) + std::string("/viking_room/viking_room.obj..."));
    m_Model->second = m_Model->first.load_model(
        std::string(AG_MODELS_PATH) + std::string("/viking_room/viking_room.obj")
    );
    AG_CORE_WARN("Creating vertex buffer for viking_room model...");
    m_VertexIndexUniformBuffers->create_vertex_buffer(
        m_Model->second.vertices,
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanColorDepthRessourcesManager,
        m_VulkanGraphicsCommandPoolManager
    );
    AG_CORE_WARN("Creating index buffer for viking_room model...");
    m_VertexIndexUniformBuffers->create_index_buffer(
        m_Model->second.indices,
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanColorDepthRessourcesManager,
        m_VulkanGraphicsCommandPoolManager
    );
    AG_CORE_WARN("Creating uniform buffers for viking_room model...");
    m_VertexIndexUniformBuffers->create_uniform_buffers(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanColorDepthRessourcesManager
    );
    AG_CORE_WARN("Creating descriptor pool...");
    m_VulkanDescriptorManager->create_descritpor_pool(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    AG_CORE_WARN("Creating descriptor sets...");
    m_VulkanDescriptorManager->create_descriptor_sets(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanTextureImageManager,
        m_VertexIndexUniformBuffers
    );
    AG_CORE_WARN("Creating command buffers...");
    m_VertexIndexUniformBuffers->create_command_buffers(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanSwapChainFrameBuffersManager,
        m_VulkanGraphicsPipelineManager,
        m_VulkanDescriptorManager,
        m_VulkanGraphicsCommandPoolManager,
        m_Model->second.indices
    );
    AG_CORE_WARN("Creating semaphores and fences...");
    m_VulkanPresenter->create_semaphores_fences_objs(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );

    return AG_SUCCESS;
}

// template <typename __Function_Signature>
// Agos::AgResult Agos::AgVulkanHandlerRenderer::run(const std::function<__Function_Signature>& to_do)
Agos::AgResult Agos::AgVulkanHandlerRenderer::run()
{
    while ( !glfwWindowShouldClose(m_GLFWInstance->get_window()) )
    {
        glfwPollEvents();
        m_VulkanPresenter->draw_frame(
            m_VulkanLogicalDevice,
            m_VulkanSwapChain,
            m_VertexIndexUniformBuffers
        );
        vkDeviceWaitIdle(m_VulkanLogicalDevice->get_device());

        // https://www.youtube.com/watch?v=NzishIREebw
        // to_do();
    }

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerRenderer::terminate_vulkan()
{
    this->terminate_swapchain();

    m_VulkanTextureImageManager->terminate();
    m_VulkanDescriptorManager->terminate_descriptor_set_layout();
    m_VertexIndexUniformBuffers->terminate_index_buffer();
    m_VertexIndexUniformBuffers->terminate_vertex_buffer();
    m_VulkanPresenter->terminate_semaphores_fences_objs();
    m_VulkanGraphicsCommandPoolManager->terminate();
    m_VulkanLogicalDevice->terminate();
    m_VertexIndexUniformBuffers->terminate();

    m_VulkanDebugLayersManager->terminate();

    m_GLFWInstance->terminate_vulkan_surface(m_VulkanInstance);
    m_VulkanInstance->destroy();
    m_GLFWInstance->terminate();

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerRenderer::terminate()
{
    if (!m_RendererTerminated)
    {
        terminate_vulkan();
        m_RendererTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

void Agos::AgVulkanHandlerRenderer::recreate_swapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_GLFWInstance->get_window(), &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_GLFWInstance->get_window(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_VulkanLogicalDevice->get_device());

    this->terminate_swapchain();

    AG_CORE_WARN("Recreating swap chain...");
    m_VulkanSwapChain->create_swap_chain(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_GLFWInstance
    );
    AG_CORE_WARN("Recreating swap chain image views...");
    m_VulkanSwapChain->create_image_views(
        m_VulkanLogicalDevice
    );
    AG_CORE_WARN("Recreating render pass...");
    m_VulkanRenderPass->create_render_pass(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    AG_CORE_WARN("Recreating graphics pipeline...");
    m_VulkanGraphicsPipelineManager->create_graphics_pipeline(
        std::string(AG_SHADERS_PATH) + '/',
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanDescriptorManager
    );
    AG_CORE_WARN("Recreating color and depths ressources...");
    m_VulkanColorDepthRessourcesManager->create_color_ressources(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    m_VulkanColorDepthRessourcesManager->create_depth_ressources(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    AG_CORE_WARN("Recreating swap chain frame buffers...");
    m_VulkanSwapChainFrameBuffersManager->create_framebuffers(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanColorDepthRessourcesManager
    );
    AG_CORE_WARN("Creating uniform buffers for viking_room model...");
    m_VertexIndexUniformBuffers->create_uniform_buffers(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanColorDepthRessourcesManager
    );
    AG_CORE_WARN("Creating descriptor pool...");
    m_VulkanDescriptorManager->create_descritpor_pool(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    AG_CORE_WARN("Creating descriptor sets...");
    m_VulkanDescriptorManager->create_descriptor_sets(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanTextureImageManager,
        m_VertexIndexUniformBuffers
    );
    AG_CORE_WARN("Creating command buffers...");
    m_VertexIndexUniformBuffers->create_command_buffers(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanSwapChainFrameBuffersManager,
        m_VulkanGraphicsPipelineManager,
        m_VulkanDescriptorManager,
        m_VulkanGraphicsCommandPoolManager,
        m_Model->second.indices
    );

    m_VulkanPresenter->get_images_in_flight().resize(m_VulkanSwapChain->get_swapchain_images().size(), VK_NULL_HANDLE);
}

void Agos::AgVulkanHandlerRenderer::terminate_swapchain()
{
    m_VulkanColorDepthRessourcesManager->terminate();
    m_VulkanSwapChainFrameBuffersManager->terminate();
    m_VertexIndexUniformBuffers->terminate_command_buffers();
    m_VulkanGraphicsPipelineManager->terminate();
    m_VulkanRenderPass->terminate();
    m_VulkanSwapChain->terminate();
    m_VertexIndexUniformBuffers->terminate_uniform_buffers();
    m_VulkanDescriptorManager->terminate_descriptor_pool();
}
