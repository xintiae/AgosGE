#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"

#include AG_EVENTBUS_INCLUDE

namespace Agos{
    class AgGLFWHandlerInstance;
}
#include "Agos/src/renderer/glfw_instance.h"
#include "Agos/src/renderer/vulkan_instance.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"
#include "Agos/src/renderer/vulkan_render_pass.h"
#include "Agos/src/renderer/vulkan_descriptor.h"
#include "Agos/src/renderer/vulkan_graphics_pipeline.h"
#include "Agos/src/renderer/vulkan_command_pool.h"
#include "Agos/src/renderer/vulkan_ressources.h"
#include "Agos/src/renderer/vulkan_framebuffers.h"
#include "Agos/src/renderer/vulkan_textures.h"
namespace Agos{
    struct AgModelLoader;
    struct AgModelData;
    struct AgModel;
}
#include "Agos/src/renderer/model_loader.h"
#include "Agos/src/renderer/vulkan_buffers.h"
namespace Agos{
    class AgVulkanHandlerPresenter;
}
#include "Agos/src/renderer/vulkan_presentation.h"

#include <functional>

namespace Agos
{
typedef class AG_API AgVulkanHandlerRenderer
{
private:
    std::shared_ptr<dexode::EventBus> m_EventBus;

    std::shared_ptr<AgGLFWHandlerEvents> m_GLFWEventsHandler;
    std::shared_ptr<AgGLFWHandlerInstance> m_GLFWInstance;
    std::shared_ptr<AgVulkanHandlerInstance> m_VulkanInstance;
    std::shared_ptr<AgVulkanHandlerDebugLayersManager> m_VulkanDebugLayersManager;

    std::shared_ptr<AgVulkanHandlerPhysicalDevice> m_VulkanPhysicalDevice;
    std::shared_ptr<AgVulkanHandlerLogicalDevice> m_VulkanLogicalDevice;
    std::shared_ptr<AgVulkanHandlerSwapChain> m_VulkanSwapChain;
    std::shared_ptr<AgVulkanHandlerRenderPass> m_VulkanRenderPass;

    std::shared_ptr<AgVulkanHandlerDescriptorManager> m_VulkanDescriptorManager;
    std::shared_ptr<AgVulkanHandlerGraphicsPipelineManager> m_VulkanGraphicsPipelineManager;
    std::shared_ptr<AgVulkanHandlerCommandPoolManager> m_VulkanGraphicsCommandPoolManager;
    std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager> m_VulkanColorDepthRessourcesManager;
    std::shared_ptr<AgVulkanHandlerFramebuffers> m_VulkanSwapChainFrameBuffersManager;

    std::vector<std::shared_ptr<AgVulkanHandlerTextureManager>> m_VulkanTextureImageManager;
    std::vector<AgModel> m_Models;
    std::shared_ptr<AgVulkanHandlerBufferManager> m_VertexIndexUniformBuffers;

    std::shared_ptr<AgVulkanHandlerPresenter> m_VulkanPresenter;


    bool m_RendererTerminated = false;

public:
    AgVulkanHandlerRenderer(const std::shared_ptr<dexode::EventBus>& event_bus);
    AgVulkanHandlerRenderer(const AgVulkanHandlerRenderer& other)   = delete;
    AgVulkanHandlerRenderer(AgVulkanHandlerRenderer&& other)        = delete;
    ~AgVulkanHandlerRenderer();

    AgVulkanHandlerRenderer& operator=(const AgVulkanHandlerRenderer& other)    = delete;
    AgVulkanHandlerRenderer& operator=(AgVulkanHandlerRenderer&& other)         = delete;

    AgResult init_vulkan(const std::vector<AgModel>& to_render_models);
    template <typename __Function_Signature>
    AgResult run(const std::function<__Function_Signature>& to_do);     // our main loop
    AgResult terminate_vulkan();
    AgResult terminate();

    friend class AgGLFWHandlerInstance;

private:
    // voids because we're throwing a std::runtime_error if something fails
    void recreate_swapchain();
    void terminate_swapchain();
    void draw_frame();
} AgVulkanHandlerRenderer;

}   // namespace Agos

/* * * * * * * template member function Agos::AgVulkanHandlerRenderer::run definition * * * * * * */

template <typename __Function_Signature>
Agos::AgResult Agos::AgVulkanHandlerRenderer::run(
    const std::function<__Function_Signature>& to_do)
{
    while ( !glfwWindowShouldClose(m_GLFWInstance->get_window()) )
    {
        this->draw_frame();
        // https://www.youtube.com/watch?v=NzishIREebw
        to_do();
    }

    return AG_SUCCESS;
}

