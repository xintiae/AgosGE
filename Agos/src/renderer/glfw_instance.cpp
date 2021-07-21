#include "Agos/src/renderer/glfw_instance.h"

#include "Agos/src/logger/logger.h"

Agos::AgGLFWHandlerInstance::AgGLFWHandlerInstance(const std::shared_ptr<dexode::EventBus>& event_bus, AgVulkanHandlerRenderer* renderer)
    : m_EventBusListener{event_bus}, m_RendererReference(renderer)
{
}

Agos::AgGLFWHandlerInstance::~AgGLFWHandlerInstance()
{
    terminate();
}

Agos::AgResult Agos::AgGLFWHandlerInstance::init(
    const std::shared_ptr<AgGLFWHandlerEvents>& event_handler)
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
    glfwSetKeyCallback(m_ApplicationWindow, event_handler->keyboardCallback);

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
    if (!m_ApplicationSurfaceTerminated)
    {
        vkDestroySurfaceKHR(vulkan_instance->get_instance(), m_ApplicationSurface, nullptr);
        m_ApplicationSurfaceTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::AgGLFWHandlerInstance::terminate()
{
    if (!m_Terminated)
    {
        glfwDestroyWindow(m_ApplicationWindow);
        glfwTerminate();

        AG_CORE_INFO("[GLFW/AgGLFWHandlerInstance - terminat] Terminated GLFW instance!");
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
        {
            m_RendererReference->m_FramebufferResized = true;
            m_RendererReference->recreate_swapchain(false);
            break;
        }
        case Agos::Events::mouseButtonCallback:
        {
            // clicky stuff over here
            break;
        }
        case Agos::Events::cursorPosCallback:
        {
            // do stuff with your mouse or something...
            break;
        }
        case Agos::Events::keyboardCallback:
        {
            // ach was für Tatsatur
            Agos::Events::AgGLFWEventKeyboardCallback* event_data = reinterpret_cast<Agos::Events::AgGLFWEventKeyboardCallback*>(event.event_data);
            Agos::AgGLFWHandlerKeyboardEventHandler::process(*event_data, m_RendererReference);
            break;
        }
        case Agos::Events::undefined:
        {
            AG_CORE_WARN("[GLFW/AgGLFWHandlerInstance - on_event_process] Tryied to process undefined event!");
            break;
        }
    }
}

void Agos::AgGLFWHandlerKeyboardEventHandler::process(
    const Agos::Events::AgGLFWEventKeyboardCallback& event_data,
    AgVulkanHandlerRenderer* renderer)
{
    switch (event_data.key)
    {
    case GLFW_KEY_W:
    {
        renderer->m_Camera->compute_all();
        renderer->m_Camera->m_CameraLastPosition = renderer->m_Camera->m_CameraPosition;
        renderer->m_Camera->m_CameraPosition += renderer->m_Camera->m_CameraSpeed * (-renderer->m_Camera->m_CameraOppositeDirection);
        renderer->m_Camera->m_CameraTarget += renderer->m_Camera->m_CameraSpeed * (-renderer->m_Camera->m_CameraOppositeDirection);
        renderer->m_Camera->compute_all();
        break;
    }
    case GLFW_KEY_S:
    {
        renderer->m_Camera->compute_all();
        renderer->m_Camera->m_CameraLastPosition = renderer->m_Camera->m_CameraPosition;
        renderer->m_Camera->m_CameraPosition -= renderer->m_Camera->m_CameraSpeed * (-renderer->m_Camera->m_CameraOppositeDirection);
        renderer->m_Camera->m_CameraTarget -= renderer->m_Camera->m_CameraSpeed * (-renderer->m_Camera->m_CameraOppositeDirection);
        renderer->m_Camera->compute_all();
        break;
    }
    case GLFW_KEY_D:
    {
        renderer->m_Camera->compute_all();
        renderer->m_Camera->m_CameraPosition += renderer->m_Camera->m_CameraRight * renderer->m_Camera->m_CameraSpeed;
        renderer->m_Camera->m_CameraTarget += renderer->m_Camera->m_CameraRight * renderer->m_Camera->m_CameraSpeed;
        renderer->m_Camera->compute_all();

        // renderer->m_Camera->compute_all();
        // renderer->m_Camera->m_CameraTarget += glm::normalize(renderer->m_Camera->m_CameraRight) * renderer->m_Camera->m_CameraSpeed;
        // renderer->m_Camera->compute_all();
        break;
    }
    case GLFW_KEY_A:
    {
        renderer->m_Camera->compute_all();
        renderer->m_Camera->m_CameraPosition -= renderer->m_Camera->m_CameraRight * renderer->m_Camera->m_CameraSpeed;
        renderer->m_Camera->m_CameraTarget -= renderer->m_Camera->m_CameraRight * renderer->m_Camera->m_CameraSpeed;
        renderer->m_Camera->compute_all();

        // renderer->m_Camera->compute_all();
        // renderer->m_Camera->m_CameraTarget -= glm::normalize(renderer->m_Camera->m_CameraRight) * renderer->m_Camera->m_CameraSpeed;
        // renderer->m_Camera->compute_all();
        break;
    }
    
    default:
        AG_CORE_INFO("[GLFW/AgGLFWHandlerKeyboardEventHandler - process] Unkown key pressed!");
        break;
    }
}
