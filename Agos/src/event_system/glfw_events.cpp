#include "Agos/src/event_system/glfw_events.h"

#include "Agos/src/logger/logger.h"

std::shared_ptr<dexode::EventBus> Agos::AgGLFWHandlerEvents::m_EventBus;

Agos::AgGLFWHandlerEvents::AgGLFWHandlerEvents(const std::shared_ptr<dexode::EventBus>& event_bus)
{
    m_EventBus = std::move(std::shared_ptr<dexode::EventBus>({event_bus}));
}

Agos::AgGLFWHandlerEvents::~AgGLFWHandlerEvents()
{
}

void Agos::AgGLFWHandlerEvents::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    // optimize?
    auto data = Agos::Events::AgGLFWEventFramebufferResizeCallback({window, width, height});
    m_EventBus->postpone<Agos::Events::AgGLFWHandlerEvent>({
        Agos::Events::AgGLFWHandlerEventType::framebufferResizeCallback,
        &data
    });
    m_EventBus->process();
}

void Agos::AgGLFWHandlerEvents::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    auto data = Agos::Events::AgGLFWEventCursorPosCallback({window, xpos, ypos});
    m_EventBus->postpone<Agos::Events::AgGLFWHandlerEvent>({
        Agos::Events::AgGLFWHandlerEventType::cursorPosCallback,
        &data
    });
    m_EventBus->process();
}

void Agos::AgGLFWHandlerEvents::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto data = Agos::Events::AgGLFWEventMouseButtonCallback({window, button, action});
    m_EventBus->postpone<Agos::Events::AgGLFWHandlerEvent>({
        Agos::Events::AgGLFWHandlerEventType::mouseButtonCallback,
        &data
    });
    m_EventBus->process();
    AG_MARK_AS_USED(mods);
}
