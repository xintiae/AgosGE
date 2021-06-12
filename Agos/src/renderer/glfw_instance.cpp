#include "Agos/src/renderer/glfw_instance.h"

#include "Agos/src/logger/logger.h"

Agos::AgGLFWHandler::AgGLFWHandler()
{
}

Agos::AgGLFWHandler::~AgGLFWHandler()
{
    this->terminate();
}

Agos::AgResult Agos::AgGLFWHandler::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_ApplicationWindow = NULL;
    if ( (m_ApplicationWindow = glfwCreateWindow(AG_WINDOW_WIDTH, AG_WINDOW_HEIGHT, "Vulkan", nullptr, nullptr)) == NULL)
    {
        AG_CORE_CRITICAL("[GLFW/init] Failed to create window!");
        return AG_FAILED_TO_CREATE_GLFW_INSTANCE;
    }

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgGLFWHandler::terminate()
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

GLFWwindow*& Agos::AgGLFWHandler::get_window()
{
    return m_ApplicationWindow;
}
