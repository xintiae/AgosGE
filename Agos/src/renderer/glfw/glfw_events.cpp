#include "Agos/src/renderer/glfw/glfw_events.h"

#include "Agos/src/logger/logger.h"

std::shared_ptr<dexode::EventBus> Agos::GLFWHandler::GLFWEvent::EventManager::m_EventBusRef;

Agos::GLFWHandler::GLFWEvent::EventManager::EventManager(const std::shared_ptr<dexode::EventBus>& event_bus)
{
    m_EventBusRef       = event_bus;
}

Agos::GLFWHandler::GLFWEvent::EventManager::~EventManager()
{
}


void Agos::GLFWHandler::GLFWEvent::EventManager::framebufferResizeCallback(
    GLFWwindow* window,
    int         width,
    int         height
)
{
    // ! gefährlich
    Agos::GLFWHandler::GLFWEvent::EventData::FramebufferResize data({window, width, height});
    m_EventBusRef->postpone<Agos::GLFWHandler::GLFWEvent::Event>({
        Agos::GLFWHandler::GLFWEvent::EventType::framebufferResizeCallback,
        // &data
        nullptr
    });
    m_EventBusRef->process();
}

void Agos::GLFWHandler::GLFWEvent::EventManager::cursorPositionCallback(
    GLFWwindow* window,
    double      xpos,
    double      ypos
)
{
    Agos::GLFWHandler::GLFWEvent::EventData::CursorPosition data({window, xpos, ypos});
    m_EventBusRef->postpone<Agos::GLFWHandler::GLFWEvent::Event>({
        Agos::GLFWHandler::GLFWEvent::EventType::cursorPosCallback,
        // &data
        nullptr
    });
    m_EventBusRef->process();
}

void Agos::GLFWHandler::GLFWEvent::EventManager::mouseButtonCallback(
    GLFWwindow* window,
    int         button,
    int         action,
    int         mods
)
{
    Agos::GLFWHandler::GLFWEvent::EventData::MouseButton data({window, button, action});
    m_EventBusRef->postpone<Agos::GLFWHandler::GLFWEvent::Event>({
        Agos::GLFWHandler::GLFWEvent::EventType::mouseButtonCallback,
        // &data
        nullptr
    });
    m_EventBusRef->process();
    AG_MARK_AS_USED(mods);
}

void Agos::GLFWHandler::GLFWEvent::EventManager::keyboardCallback(
    GLFWwindow* window,
    int         key,
    int         scancode,
    int         action,
    int         mods
)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        Agos::GLFWHandler::GLFWEvent::EventData::Keyboard data ({window, key, scancode, action, mods});
        m_EventBusRef->postpone<Agos::GLFWHandler::GLFWEvent::Event>({
            Agos::GLFWHandler::GLFWEvent::EventType::keyboardCallback,
            // &data
            nullptr
        });
        m_EventBusRef->process();
    }
}
