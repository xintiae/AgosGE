#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"
#include "Agos/src/renderer/renderer.h"
#include "Agos/src/debug_layers/vulkan_debug_layers.h"

#include AG_EVENTBUS_INCLUDE

namespace Agos
{
typedef class AG_API AgApplication
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
    
    std::shared_ptr<AgVulkanHandlerTextureManager> m_VulkanTextureImageManager;
    std::shared_ptr<std::pair<AgModelLoader, AgVertexIndexHolder>> m_VikingRoomModel;

public:
    AgApplication();
    virtual ~AgApplication();

    AgResult core_init_application();
    AgResult core_run_application();
    AgResult core_terminate_application();

    virtual AgResult client_init_application()      = 0;
    virtual AgResult client_run_application()       = 0;
    virtual AgResult client_terminate_application() = 0;
} AgApplication; 

}   // namespace Agos