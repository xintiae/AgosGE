#include "Agos/src/renderer/renderer.h"

#include "Agos/src/renderer/imgui/mandatories/imgui_impl_vulkan.h"
#include "Agos/src/logger/logger.h"
#include <chrono>
#include <tuple>

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
    m_VulkanGraphicsPipelineManager      = std::make_shared<AgVulkanHandlerGraphicsPipelineManager>();
    m_VulkanGraphicsCommandPoolManager   = std::make_shared<AgVulkanHandlerCommandPoolManager>();
    m_VulkanColorDepthRessourcesManager  = std::make_shared<AgVulkanHandlerColorDepthRessourcesManager>();
    m_VulkanCommandBuffer                = std::make_shared<AgVulkanHandlerCommandBufferManager>();

    m_VulkanPresenter   = std::make_shared<AgVulkanHandlerPresenter>();
    m_Camera            = std::make_shared<AgCameraObject>(glm::vec3(-5.0f, 5.0f, 5.0f), -glm::vec3(1.0f, -1.0f, -1.0f));
    m_ImGui             = std::make_shared<AgImGuiHandler>(m_GLFWInstance->get_window());

    m_VulkanDescriptorManager            = std::make_shared<AgVulkanHandlerDescriptorManager>();
}

Agos::AgVulkanHandlerRenderer::~AgVulkanHandlerRenderer()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerRenderer::init_vulkan(const std::vector<AgModel>& to_render_models, const bool& should_cursor_exist)
{
    m_Models = std::move(to_render_models);
    m_VulkanTextureImageManager.resize(m_Models.size());
    for (size_t i = 0; i < m_VulkanTextureImageManager.size(); i++)
    {
        m_VulkanTextureImageManager[i] = std::make_shared<AgVulkanHandlerTextureManager>();
    }
    m_VertexIndexUniformBuffers.resize(m_Models.size());
    for (size_t i = 0; i < m_VertexIndexUniformBuffers.size(); i++)
    {
        m_VertexIndexUniformBuffers[i] = std::make_shared<AgVulkanHandlerVIUBufferManager>();
    }

    AG_CORE_WARN("Initializing GLFW instance...");
    m_GLFWInstance->init(m_GLFWEventsHandler, should_cursor_exist);

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
        m_VulkanLogicalDevice
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
    m_VulkanSwapChain->create_framebuffers(
        m_VulkanLogicalDevice,
        m_VulkanRenderPass,
        m_VulkanColorDepthRessourcesManager
    );

    for (size_t i = 0; i < m_Models.size(); i++)
    {
        AG_CORE_WARN("Loading model : \"" + m_Models[i].id + "\" textures...");
        m_VulkanTextureImageManager[i]->create_texture_image(
            m_Models[i].path_to_texture_file,
            m_VulkanPhysicalDevice,
            m_VulkanLogicalDevice,
            m_VulkanColorDepthRessourcesManager,
            m_VulkanGraphicsCommandPoolManager
        );
        m_VulkanTextureImageManager[i]->create_texture_image_view(
            m_VulkanLogicalDevice,
            m_VulkanSwapChain
        );
        m_VulkanTextureImageManager[i]->create_texture_sampler(
            m_VulkanPhysicalDevice,
            m_VulkanLogicalDevice
        );
        AG_CORE_WARN("Creating model : \"" + m_Models[i].id + "\" vertex buffer...");
        m_VertexIndexUniformBuffers[i]->create_vertex_buffer(
            m_Models[i].model_data.vertices,
            m_VulkanPhysicalDevice,
            m_VulkanLogicalDevice,
            m_VulkanColorDepthRessourcesManager,
            m_VulkanGraphicsCommandPoolManager
        );
        AG_CORE_WARN("Creating model : \"" + m_Models[i].id + "\" index buffer...");
        m_VertexIndexUniformBuffers[i]->create_index_buffer(
            m_Models[i].model_data.indices,
            m_VulkanPhysicalDevice,
            m_VulkanLogicalDevice,
            m_VulkanColorDepthRessourcesManager,
            m_VulkanGraphicsCommandPoolManager
        );
        AG_CORE_WARN("Creating model : \"" + m_Models[i].id + "\" uniform buffers...");
        m_VertexIndexUniformBuffers[i]->create_uniform_buffers(
            m_VulkanPhysicalDevice,
            m_VulkanLogicalDevice,
            m_VulkanSwapChain,
            m_VulkanColorDepthRessourcesManager
        );
    }
    AG_CORE_WARN("Creating descriptor pool...");
    m_VulkanDescriptorManager->create_descritpor_pool(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    for (size_t i = 0; i < m_Models.size(); i++)
    {
        AG_CORE_WARN("Creating descriptor sets for model : \"" + m_Models[i].id + "\"...");
        m_VulkanDescriptorManager->create_descriptor_sets(
            m_VulkanLogicalDevice,
            m_VulkanSwapChain,
            m_VulkanTextureImageManager[i],
            m_VertexIndexUniformBuffers[i],
            i
        );
    }
    AG_CORE_WARN("Creating command buffers...");
    m_VulkanCommandBuffer->create_command_buffers(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanGraphicsPipelineManager,
        m_VulkanGraphicsCommandPoolManager,
        m_VulkanDescriptorManager,
        m_VertexIndexUniformBuffers,
        m_Models
    );
    AG_CORE_WARN("Creating semaphores and fences...");
    m_VulkanPresenter->create_semaphores_fences_objs(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );

    AG_CORE_WARN("Initializing ImGui...");
    m_ImGui->init(
        m_GLFWInstance,
        m_VulkanInstance,
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanGraphicsCommandPoolManager
    );

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerRenderer::update_models_data(const std::vector<Agos::AgModel>& to_update_models, const bool& keep_informed)
{
    std::vector<std::pair<bool, size_t>> models_match( to_update_models.size(), std::move(std::pair<bool, size_t>(false, 0)) );
    bool doesnt_match = false;

    for (size_t i = 0; i < to_update_models.size(); i++)
    {
        for (size_t y = 0; y < m_Models.size(); y++)
        {
            if (to_update_models[i].id == m_Models[y].id)
            {
                models_match[i] = (std::move(std::pair<bool, size_t>(true, y)));
                break;
            }
        }
    }

    for (const std::pair<bool, size_t>& match : models_match)
    {
        if (match.first == false)
            doesnt_match = true;
    }

    if (doesnt_match)
    {
        AG_CORE_WARN("[Vulkan/AgVulkanHandlerRenderer - update_models_data] Models specified for update don't match given models during renderer's initializing op!");
        AG_CORE_WARN("[Vulkan/AgVulkanHandlerRenderer - update_models_data] Abording models' update...");
        return AG_UPDATE_MODELS_DONT_MATCH;
    }

    for (size_t i = 0; i < models_match.size(); i++)
    {
        m_Models[models_match[i].second].model_data = std::move(to_update_models[i].model_data);

        if (m_Models[models_match[i].second].extension_type == Agos::AgModelExtensionDataType::light_source)
        {
            Agos::AgModelExtensionDataLight* extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(m_Models[models_match[i].second].pExtensionData);
            extension_data->light_position = m_Models[models_match[i].second].model_data.vertices[0].pos;
        }
    }


    // ! ======================= MOVE INTO HELPERS FUNCTIONS
    for (size_t i = 0; i < models_match.size(); i++)
    {
        m_VulkanCommandBuffer->terminate_command_buffers(false, false);
        if (keep_informed)
        {
            AG_CORE_INFO("Updating model : \"" + m_Models[i].id + "\" vertex buffer...");
        }
        m_VertexIndexUniformBuffers[models_match[i].second]->update_vertex_buffer(
            m_Models[models_match[i].second].model_data.vertices,
            m_VulkanPhysicalDevice,
            m_VulkanLogicalDevice,
            m_VulkanColorDepthRessourcesManager,
            m_VulkanGraphicsCommandPoolManager,
            keep_informed
        );
        if (keep_informed)
        {
            AG_CORE_INFO("Updating model : \"" + m_Models[i].id + "\" index buffer...");
        }
        m_VertexIndexUniformBuffers[models_match[i].second]->update_index_buffer(
            m_Models[models_match[i].second].model_data.indices,
            m_VulkanPhysicalDevice,
            m_VulkanLogicalDevice,
            m_VulkanColorDepthRessourcesManager,
            m_VulkanGraphicsCommandPoolManager,
            keep_informed
        );
    }
    if (keep_informed)
    {
        AG_CORE_INFO("Updating command buffers...");
    }
    m_VulkanCommandBuffer->update_command_buffers(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanGraphicsPipelineManager,
        m_VulkanGraphicsCommandPoolManager,
        m_VulkanDescriptorManager,
        m_VertexIndexUniformBuffers,
        m_Models,
        m_ImGui,
        keep_informed
    );
    // ! ======================= MOVE INTO HELPER FUNCTION

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerRenderer::run()
{
    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
    glfwPollEvents();

    m_VulkanPresenter->draw_frame(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VertexIndexUniformBuffers,
        m_Models,
        m_VulkanCommandBuffer,
        this
    );
    vkDeviceWaitIdle(m_VulkanLogicalDevice->get_device());

    this->m_Camera->calculate_adequate_camera_speed(start_time);
    return AG_SUCCESS;
}

Agos::AgBool Agos::AgVulkanHandlerRenderer::can_run()
{
    return !glfwWindowShouldClose(m_GLFWInstance->get_window());
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

Agos::AgResult Agos::AgVulkanHandlerRenderer::terminate_vulkan()
{
    this->terminate_swapchain(true);

    for (size_t i = 0; i < m_VulkanTextureImageManager.size(); i++)
    {
        m_VulkanTextureImageManager[i]->terminate();
    }

    m_VulkanDescriptorManager->terminate_descriptor_set_layout();
    for (size_t i = 0; i < m_VertexIndexUniformBuffers.size(); i++)
    {
        m_VertexIndexUniformBuffers[i]->terminate_index_buffer();
        m_VertexIndexUniformBuffers[i]->terminate_vertex_buffer();
    }
    m_VulkanPresenter->terminate_semaphores_fences_objs();
    m_VulkanGraphicsCommandPoolManager->terminate();
    m_VulkanLogicalDevice->terminate();
    for (size_t i = 0; i < m_VertexIndexUniformBuffers.size(); i++)
    {
        m_VertexIndexUniformBuffers[i]->terminate();
    }

    m_VulkanDebugLayersManager->terminate();

    m_GLFWInstance->terminate_vulkan_surface(m_VulkanInstance);
    m_VulkanInstance->destroy();
    m_GLFWInstance->terminate();

    return AG_SUCCESS;
}

void Agos::AgVulkanHandlerRenderer::recreate_swapchain(const bool& mark_instances_terminated)
{
    if (!m_ResizeOpLocked)
    {
        m_ResizeOpLocked = true;
        do_it(mark_instances_terminated);
        std::this_thread::sleep_for(std::chrono::milliseconds(AG_SWAPCHAIN_RECREATION_LOCK_TIME_MS));
        m_ResizeOpLocked = false;
    }
}

void Agos::AgVulkanHandlerRenderer::do_it(const bool& mark_instances_terminated)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_GLFWInstance->get_window(), &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_GLFWInstance->get_window(), &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(m_VulkanLogicalDevice->get_device());

    this->terminate_swapchain(mark_instances_terminated);

    AG_CORE_INFO("Recreating swap chain...");
    m_VulkanSwapChain->create_swap_chain(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_GLFWInstance
    );
    AG_CORE_INFO("Recreating swap chain image views...");
    m_VulkanSwapChain->create_image_views(
        m_VulkanLogicalDevice
    );
    AG_CORE_INFO("Recreating render pass...");
    m_VulkanRenderPass->create_render_pass(
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );
    AG_CORE_INFO("Recreating graphics pipeline...");
    m_VulkanGraphicsPipelineManager->create_graphics_pipeline(
        std::string(AG_SHADERS_PATH) + '/',
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanDescriptorManager
    );
    AG_CORE_INFO("Recreating color and depths ressources...");
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
    AG_CORE_INFO("Recreating swap chain frame buffers...");
    m_VulkanSwapChain->create_framebuffers(
        m_VulkanLogicalDevice,
        m_VulkanRenderPass,
        m_VulkanColorDepthRessourcesManager
    );
    for (size_t i = 0; i < m_VertexIndexUniformBuffers.size(); i++)
    {
        AG_CORE_INFO("Recreating uniform buffers for for model : \"" + m_Models[i].id + "\"...");
        m_VertexIndexUniformBuffers[i]->create_uniform_buffers(
            m_VulkanPhysicalDevice,
            m_VulkanLogicalDevice,
            m_VulkanSwapChain,
            m_VulkanColorDepthRessourcesManager
        );
    }
    AG_CORE_INFO("Recreating descriptor pool...");
    m_VulkanDescriptorManager->create_descritpor_pool(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain
    );

    for (size_t i = 0; i < m_VulkanTextureImageManager.size(); i++)
    {
        AG_CORE_INFO("Recreating descriptor sets for model : \"" + m_Models[i].id + "\"...");
        m_VulkanDescriptorManager->create_descriptor_sets(
            m_VulkanLogicalDevice,
            m_VulkanSwapChain,
            m_VulkanTextureImageManager[i],
            m_VertexIndexUniformBuffers[i],
            i
        );
    }
    AG_CORE_INFO("Recreating command buffers...");
    m_VulkanCommandBuffer->create_command_buffers(
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanGraphicsPipelineManager,
        m_VulkanGraphicsCommandPoolManager,
        m_VulkanDescriptorManager,
        m_VertexIndexUniformBuffers,
        m_Models
    );

    m_VulkanPresenter->get_images_in_flight().resize(m_VulkanSwapChain->get_swapchain_images().size(), VK_NULL_HANDLE);

    AG_CORE_INFO("Reinitializing ImGui...");
    m_ImGui->init(
        m_GLFWInstance,
        m_VulkanInstance,
        m_VulkanPhysicalDevice,
        m_VulkanLogicalDevice,
        m_VulkanSwapChain,
        m_VulkanRenderPass,
        m_VulkanGraphicsCommandPoolManager
    );
}

void Agos::AgVulkanHandlerRenderer::terminate_swapchain(const bool& mark_instances_terminated)
{
    m_ImGui->terminate(mark_instances_terminated);
    m_VulkanColorDepthRessourcesManager->terminate(mark_instances_terminated);
    m_VulkanSwapChain->terminate_framebuffers(mark_instances_terminated);
    m_VulkanCommandBuffer->terminate_command_buffers(mark_instances_terminated);
    m_VulkanGraphicsPipelineManager->terminate(mark_instances_terminated);
    m_VulkanRenderPass->terminate(mark_instances_terminated);
    m_VulkanSwapChain->terminate(mark_instances_terminated);
    for (size_t i = 0; i < m_VertexIndexUniformBuffers.size(); i++)
    {
        m_VertexIndexUniformBuffers[i]->terminate_uniform_buffers(mark_instances_terminated);
    }
    m_VulkanDescriptorManager->terminate_descriptor_pool(mark_instances_terminated);
}
