#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"

#include AG_EVENTBUS_INCLUDE
#include AG_GLFW_INCLUDE
#include AG_VULKAN_INCLUDE

#include <thread>

// see file Agos/src/renderer/vulkan_buffers.cpp
// static VkDevice        AG_DEFAULT_LOGICAL_DEVICE_REFERENCE = VK_NULL_HANDLE;
// static VkCommandPool   AG_DEFAULT_COMMAND_POOL_REFERENCE = VK_NULL_HANDLE;


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
#include "Agos/src/renderer/vulkan_textures.h"
#include "Agos/src/renderer/vulkan_buffers.h"
namespace Agos{
    class AgVulkanHandlerPresenter;
}
#include "Agos/src/renderer/vulkan_presentation.h"
namespace Agos{
    struct AgModelLoader;
    struct AgModelData;
    struct AgModel;
}
#include "Agos/src/renderer/modeling/model.h"
#include "Agos/src/renderer/camera.h"
namespace Agos{
    class AgImGuiHandler;
}
#include "Agos/src/renderer/imgui/agos_imgui.h"


namespace Agos
{
class AG_API AgVulkanHandlerRenderer
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
    std::shared_ptr<AgVulkanHandlerGraphicsPipelineManager> m_VulkanGraphicsPipelineManager;
    std::shared_ptr<AgVulkanHandlerCommandPoolManager> m_VulkanGraphicsCommandPoolManager;
    std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager> m_VulkanColorDepthRessourcesManager;

    std::shared_ptr<AgVulkanHandlerCommandBufferManager> m_VulkanCommandBuffer;
    std::shared_ptr<AgVulkanHandlerPresenter> m_VulkanPresenter;
    std::shared_ptr<AgImGuiHandler> m_ImGui;


    std::shared_ptr<AgVulkanHandlerDescriptorManager> m_VulkanDescriptorManager;
    // - - - all those three helpers are models-dependent - - -
    std::vector<std::shared_ptr<AgVulkanHandlerTextureManager>> m_VulkanTextureImageManager;
    std::vector<AgModel> m_Models;
    std::vector<std::shared_ptr<AgVulkanHandlerVIUBufferManager>> m_VertexIndexUniformBuffers;
    //  - - - ===  - - -


    bool m_FramebufferResized   = false;
    bool m_ResizeOpLocked       = false;
    bool m_RendererTerminated   = false;

public:
    AgVulkanHandlerRenderer(const std::shared_ptr<dexode::EventBus>& event_bus);
    AgVulkanHandlerRenderer(const AgVulkanHandlerRenderer& other)   = delete;
    AgVulkanHandlerRenderer(AgVulkanHandlerRenderer&& other)        = delete;
    ~AgVulkanHandlerRenderer();

    AgVulkanHandlerRenderer& operator=(const AgVulkanHandlerRenderer& other)    = delete;
    AgVulkanHandlerRenderer& operator=(AgVulkanHandlerRenderer&& other)         = delete;

    AgResult    init_vulkan(const std::vector<AgModel>& to_render_models, const bool& should_cursor_exist = false);
    AgResult    run();
    AgBool      can_run();
    /**
     * @brief main function to update the models' data given when initializing the renderer
     * @param to_update_models an array of models to update |
     * you don't have to specify here all your models; you can pass here just the ones you updated (those ones will be identified using their id)
    */
    AgResult update_models_data(const std::vector<AgModel>& to_update_models, const bool& keep_informed = false);
    AgResult terminate();
    AgResult terminate_vulkan();

    friend class AgGLFWHandlerInstance;
    friend struct AgGLFWHandlerKeyboardEventHandler;
    friend struct AgGLFWHandlerCursorPosEventHandler;
    friend class AgVulkanHandlerPresenter;
    friend class AgImGuiHandler;

protected:
    std::shared_ptr<AgCameraObject> m_Camera;

    std::unique_ptr<std::thread> m_ResizeGuardian;

    // voids because we're throwing a std::runtime_error if something fails
    void recreate_swapchain(const bool& mark_instances_terminated = false);
    void do_it(const bool& mark_instances_terminated = false);

private:
    void terminate_swapchain(const bool& mark_instances_terminated = true);
};  // class AgVulkanHandlerRenderer

}   // namespace Agos
