#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"
#include "Agos/src/debug_layers/debug_layers.h"
#include "Agos/src/debug_layers/vulkan_debug_layers.h"


namespace Agos
{
typedef class AG_API AgApplication
{
private:
    std::shared_ptr<AgVulkanHandlerInstance> m_VulkanInstance;
    std::shared_ptr<AgVulkanHandlerDebugLayersManager> m_VulkanDebugLayersManager;

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