#include "Agos/src/entry_point/entry_point.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/debug_layers/debug_layers.h"
#include "Agos/src/renderer/renderer.h"

Agos::AgApplication::AgApplication()
{
    m_EventBus                  = std::make_shared<dexode::EventBus>();
    m_GLFWEventsHandler         = std::make_shared<AgGLFWHandlerEvents>(m_EventBus);
    m_GLFWInstance              = std::make_shared<AgGLFWHandlerInstance>(m_EventBus);
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

    m_VikingRoomModel               = std::make_shared<std::pair<AgModelLoader, AgVertexIndexHolder>>();
    m_VertexIndexUniformBuffers     = std::make_shared<AgVulkanHandlerBufferManager>();
}

Agos::AgApplication::~AgApplication()
{
}

Agos::AgResult Agos::AgApplication::core_init_application()
{
    Agos::ag_init_loggers();
    AG_CORE_WARN("Initializing Agos core application...");

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
    m_VikingRoomModel->second = m_VikingRoomModel->first.load_model(
        std::string(AG_MODELS_PATH) + std::string("/viking_room/viking_room.obj")
    );
    AG_CORE_WARN("Creating vertex buffer for viking_room model...");
    m_VertexIndexUniformBuffers->create_vertex_buffer(
        m_VikingRoomModel->second.vertices,
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanColorDepthRessourcesManager,
        m_VulkanGraphicsCommandPoolManager
    );
    AG_CORE_WARN("Creating index buffer for viking_room model...");
    m_VertexIndexUniformBuffers->create_index_buffer(
        m_VikingRoomModel->second.indices,
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
        m_VikingRoomModel->second.indices
    );

    AG_CORE_INFO("Done initializing Agos core application!");
    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::AgApplication::core_run_application()
{
    AG_CORE_WARN("Running Agos core application...");

    while ( !glfwWindowShouldClose(m_GLFWInstance->get_window()) )
    {
        glfwPollEvents();
    }


    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::AgApplication::core_terminate_application()
{
    AG_CORE_WARN("Terminating Agos core application...");

    m_VulkanColorDepthRessourcesManager->terminate();
    m_VulkanSwapChainFrameBuffersManager->terminate();
    m_VertexIndexUniformBuffers->terminate_command_buffers();
    m_VulkanGraphicsPipelineManager->terminate();
    m_VulkanRenderPass->terminate();
    m_VulkanSwapChain->terminate();
    m_VertexIndexUniformBuffers->terminate_uniform_buffers();
    m_VulkanDescriptorManager->terminate_descriptor_pool();

    m_VulkanTextureImageManager->terminate();
    m_VulkanDescriptorManager->terminate_descriptor_set_layout();
    m_VertexIndexUniformBuffers->terminate_index_buffer();
    m_VertexIndexUniformBuffers->terminate_vertex_buffer();
    // semaphores
    // fences
    m_VulkanGraphicsCommandPoolManager->terminate();
    m_VulkanLogicalDevice->terminate();

    m_VulkanDebugLayersManager->terminate();

    m_GLFWInstance->terminate_vulkan_surface(m_VulkanInstance);
    m_VulkanInstance->destroy();
    m_GLFWInstance->terminate();

    AG_CORE_INFO("Terminated Agos core application!");
    AG_CORE_WARN("Exiting...");
    return Agos::AG_SUCCESS;
}
