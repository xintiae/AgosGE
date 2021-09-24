#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"

namespace Agos{
    namespace VulkanHandler { class VulkanBase;   }
    namespace GLFWHandler {
        class GLFWInstance;
        namespace Event { class EventManager; }
    }
}
#include "Agos/src/renderer/renderer.h"
#include AG_EVENTBUS_INCLUDE

namespace Agos
{
typedef class AG_API AgApplication
{
private:
    std::shared_ptr<dexode::EventBus> m_EventBus;
    // std::shared_ptr<AgVulkanHandlerRenderer> m_Renderer;
    // std::vector<Agos::AgModel> m_Rendered_Models;

    // ! TESTING
    std::shared_ptr<Agos::GLFWHandler::Event::EventManager> m_GLFWEventManager;
    std::shared_ptr<Agos::GLFWHandler::GLFWInstance> m_GLFWInstance;
    std::shared_ptr<Agos::VulkanHandler::VulkanBase> m_VulkanBase;

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