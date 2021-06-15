#include "Agos/src/event_system/glfw_events.h"

#include "Agos/src/logger/logger.h"

Agos::AgGLFWHandlerEvents::AgGLFWHandlerEvents(const std::shared_ptr<dexode::EventBus>& event_bus)
    // : m_EventBus(event_bus)
{
    m_EventBus = event_bus;
}

Agos::AgGLFWHandlerEvents::~AgGLFWHandlerEvents()
{
}

void Agos::AgGLFWHandlerEvents::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    m_EventBus->postpone<AgGLFWHandlerEventTypes::glfwFramebufferResizeCallback>({
        AgGLFWHandlerEventTypes::type::framebufferResizeCallback,
        window, width, height});
}

void Agos::AgGLFWHandlerEvents::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    m_EventBus->postpone<AgGLFWHandlerEventTypes::glfwCursorPosCallback>({
        AgGLFWHandlerEventTypes::type::cursorPosCallback,
        window, xpos, ypos});
}

void Agos::AgGLFWHandlerEvents::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action  == GLFW_PRESS)
    {
        AG_CORE_INFO("[GLFW/mouseButtonCallback] Triggered event : mouse left button pressed!");        
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action  == GLFW_RELEASE)
    {
        AG_CORE_INFO("[GLFW/mouseButtonCallback] Triggered event : mouse left button pressed!");        
    }
    m_EventBus->postpone<AgGLFWHandlerEventTypes::tglfwMouseButtonCallback>({
        AgGLFWHandlerEventTypes::type::mouseButtonCallback,
        window, button, action});
}
