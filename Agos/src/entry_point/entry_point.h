#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"
#include "Agos/src/event_system/glfw_events.h"
#include "Agos/src/renderer/glfw_instance.h"
#include "Agos/src/debug_layers/vulkan_debug_layers.h"
#include "Agos/src/renderer/vulkan_physical_device.h"

#include <Agos/vendor/EventBus/lib/src/dexode/EventBus.hpp>

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