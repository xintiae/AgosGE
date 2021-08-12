#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"
#include "Agos/src/renderer/renderer.h"
#include "Agos/src/debug_layers/debug_layers.h"

#include AG_EVENTBUS_INCLUDE
#include <functional>

namespace Agos
{
typedef class AG_API AgApplication
{
private:
    std::shared_ptr<dexode::EventBus> m_EventBus;
    std::shared_ptr<AgVulkanHandlerRenderer> m_Renderer;
    std::vector<Agos::AgModel> m_Rendered_Models;

public:
    AgApplication();
    virtual ~AgApplication();

    AgResult core_init_application();
    AgResult core_run_application();
    AgResult core_terminate_application();

    virtual AgResult client_init_application()      = 0;
    virtual AgResult client_run_application()       = 0;
    virtual AgResult client_terminate_application() = 0;

// example_sake
private:
    void load_models();
} AgApplication; 

}   // namespace Agos