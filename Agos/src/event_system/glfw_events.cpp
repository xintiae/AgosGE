#include "Agos/src/event_system/glfw_events.h"

#include "Agos/src/logger/logger.h"

std::shared_ptr<dexode::EventBus> Agos::GLFWHandler::Event::EventManager::m_EventBus;

Agos::GLFWHandler::Event::EventManager::EventManager()
{
}

Agos::GLFWHandler::Event::EventManager::EventManager(const std::shared_ptr<dexode::EventBus>& event_bus)
{
    m_EventBus = event_bus;
}

Agos::GLFWHandler::Event::EventManager::~EventManager()
{
}

std::shared_ptr<dexode::EventBus>& Agos::GLFWHandler::Event::EventManager::get_event_bus()
{
    return m_EventBus;
}

void Agos::GLFWHandler::Event::EventManager::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    // optimize?
    Agos::GLFWHandler::Event::Callbacks::FramebufferResize data({window, width, height});
    m_EventBus->postpone<Agos::GLFWHandler::Event::Event>({
        Agos::GLFWHandler::Event::Type::framebufferResizeCallback,
        &data
    });
    m_EventBus->process();
}

void Agos::GLFWHandler::Event::EventManager::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    Agos::GLFWHandler::Event::Callbacks::CursorPosition data({window, xpos, ypos});
    m_EventBus->postpone<Agos::GLFWHandler::Event::Event>({
        Agos::GLFWHandler::Event::Type::cursorPosCallback,
        &data
    });
    m_EventBus->process();
}

void Agos::GLFWHandler::Event::EventManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Agos::GLFWHandler::Event::Callbacks::MouseButton data({window, button, action});
    m_EventBus->postpone<Agos::GLFWHandler::Event::Event>({
        Agos::GLFWHandler::Event::Type::mouseButtonCallback,
        &data
    });
    m_EventBus->process();
    AG_MARK_AS_USED(mods);
}

void Agos::GLFWHandler::Event::EventManager::keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        Agos::GLFWHandler::Event::Callbacks::Keyboard data ({window, key, scancode, action, mods});
        m_EventBus->postpone<Agos::GLFWHandler::Event::Event>({
            Agos::GLFWHandler::Event::Type::keyboardCallback,
            &data
        });
        m_EventBus->process();
    }
}
