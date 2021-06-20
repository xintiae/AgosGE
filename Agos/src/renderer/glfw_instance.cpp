#include "Agos/src/renderer/glfw_instance.h"

#include "Agos/src/logger/logger.h"

Agos::AgGLFWHandlerInstance::AgGLFWHandlerInstance(const std::shared_ptr<dexode::EventBus>& event_bus)
    : m_EventBusListener{event_bus}
{
}

Agos::AgGLFWHandlerInstance::~AgGLFWHandlerInstance()
{
    this->terminate();
}

Agos::AgResult Agos::AgGLFWHandlerInstance::init(const std::shared_ptr<AgGLFWHandlerEvents>& event_handler)
{
    m_EventBusListener.listen<Agos::Events::AgGLFWHandlerEvent>(
        [this](const Agos::Events::AgGLFWHandlerEvent& event) -> void
        {
            this->on_event_process(event);
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
    glfwSetFramebufferSizeCallback(m_ApplicationWindow, event_handler->framebufferResizeCallback);

    glfwSetMouseButtonCallback(m_ApplicationWindow, event_handler->mouseButtonCallback);
    glfwSetCursorPosCallback(m_ApplicationWindow, event_handler->cursorPosCallback);

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgGLFWHandlerInstance::setup_vulkan_surface(const std::shared_ptr<AgVulkanHandlerInstance>& vulkan_instance)
{
    if (glfwCreateWindowSurface(vulkan_instance->get_instance(), m_ApplicationWindow, nullptr, &m_ApplicationSurface) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[GLFW/HandlerInstance] failed to create vulkan window surface!");
        return AG_FAILED_TO_CREATE_WINDOW_SURFACE;
    }
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgGLFWHandlerInstance::terminate_vulkan_surface(const std::shared_ptr<AgVulkanHandlerInstance>& vulkan_instance)
{
    vkDestroySurfaceKHR(vulkan_instance->get_instance(), m_ApplicationSurface, nullptr);
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

VkSurfaceKHR& Agos::AgGLFWHandlerInstance::get_surface()
{
    return m_ApplicationSurface;
}

void Agos::AgGLFWHandlerInstance::on_event_process(const Agos::Events::AgGLFWHandlerEvent& event)
{
    switch (event.type)
    {
    case Agos::Events::framebufferResizeCallback:
        // resize rendering and swapchain...
        break;
    
    case Agos::Events::mouseButtonCallback:
        // clicky stuff over here
        break;
    
    case Agos::Events::cursorPosCallback:
        // do stuff with your mouse or something...
        break;

    default:
        AG_CORE_WARN("[GLFW/HandlerInstance] noticed unkown event");
        break;
    }
}
