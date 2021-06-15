#include "Agos/src/renderer/glfw_instance.h"

#include "Agos/src/logger/logger.h"

Agos::AgGLFWHandlerInstance::AgGLFWHandlerInstance(const std::shared_ptr<dexode::EventBus>& event_bus)
    : m_EventBusListener(event_bus)
{
}

Agos::AgGLFWHandlerInstance::~AgGLFWHandlerInstance()
{
    this->terminate();
}

Agos::AgResult Agos::AgGLFWHandlerInstance::init(const std::shared_ptr<AgGLFWHandlerEvents>& eventHandler)
{
    m_EventBusListener.listen<AgGLFWHandlerEvents>(
        [this](const AgGLFWHandlerEvents& event) -> void
        {
            AG_CORE_INFO("[GLFW/AgGLFWHandlerInstance - init] Event notified!");
        }
    );

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_ApplicationWindow = NULL;
    if ( (m_ApplicationWindow = glfwCreateWindow(AG_WINDOW_WIDTH, AG_WINDOW_HEIGHT, "Vulkan", nullptr, nullptr)) == NULL)
    {
        AG_CORE_CRITICAL("[GLFW/init] Failed to create window!");
        return AG_FAILED_TO_CREATE_GLFW_INSTANCE;
    }

    glfwSetWindowUserPointer(m_ApplicationWindow, this);
    glfwSetFramebufferSizeCallback(m_ApplicationWindow, eventHandler->framebufferResizeCallback);

    glfwSetMouseButtonCallback(m_ApplicationWindow, eventHandler->mouseButtonCallback);
    glfwSetCursorPosCallback(m_ApplicationWindow, eventHandler->cursorPosCallback);

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgGLFWHandlerInstance::terminate()
{
    if (!m_Terminated)
    {
        glfwDestroyWindow(m_ApplicationWindow);
        glfwTerminate();
        m_Terminated = true;
        
        return AG_SUCCESS;
    }

    return AG_INSTANCE_ALREADY_TERMINATED;
}

GLFWwindow*& Agos::AgGLFWHandlerInstance::get_window()
{
    return m_ApplicationWindow;
}
