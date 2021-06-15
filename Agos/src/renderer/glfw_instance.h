#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"
#include "Agos/src/event_system/glfw_events.h"

#include <Agos/vendor/EventBus/lib/src/dexode/EventBus.hpp>
#include <GLFW/glfw3.h>
#include <memory>

namespace Agos
{
typedef class AG_API AgGLFWHandlerInstance
{
private:
    // std::shared_ptr<dexode::EventBus::Listener> m_EventBusListener;
    dexode::EventBus::Listener m_EventBusListener;
    GLFWwindow* m_ApplicationWindow;
    bool m_Terminated = false;

    bool m_framebufferResized = false;


public:
    AgGLFWHandlerInstance(const std::shared_ptr<dexode::EventBus>& event_bus);
    ~AgGLFWHandlerInstance();

    AgResult init(const std::shared_ptr<AgGLFWHandlerEvents>& eventHandler);
    AgResult terminate();

    GLFWwindow*& get_window();

private:
} AgGLFWHandlerInstance;

} // namespace Agos
