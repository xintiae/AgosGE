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
    const std::shared_ptr<AgGLFWHandlerEvents>& event_handler,
    const bool& shall_cursor_exist)
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
    if ( (m_ApplicationWindow = glfwCreateWindow(AG_MAX_WINDOW_WIDTH, AG_MAX_WINDOW_HEIGHT, "Vulkan", nullptr, nullptr)) == NULL)
    {
        AG_CORE_CRITICAL("[GLFW/init] Failed to create window!");
        return AG_FAILED_TO_CREATE_GLFW_INSTANCE;
    }

    glfwSetWindowTitle(m_ApplicationWindow, "AgosGE - init example!");

    glfwSetWindowUserPointer(m_ApplicationWindow, this);
    glfwSetFramebufferSizeCallback(m_ApplicationWindow, event_handler->framebufferResizeCallback);

    glfwSetMouseButtonCallback(m_ApplicationWindow, event_handler->mouseButtonCallback);
    glfwSetCursorPosCallback(m_ApplicationWindow, event_handler->cursorPosCallback);

    if (shall_cursor_exist)
    {
        glfwSetInputMode(m_ApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_CursorState = GLFW_CURSOR_NORMAL;
    }
    else
    {
        glfwSetInputMode(m_ApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_CursorState = GLFW_CURSOR_DISABLED;
    }
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

size_t& Agos::AgGLFWHandlerInstance::get_cursor_state()
{
    return m_CursorState;
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
            if (this->m_CursorState == GLFW_CURSOR_DISABLED)
            {
                Agos::Events::AgGLFWEventCursorPosCallback* event_data = reinterpret_cast<Agos::Events::AgGLFWEventCursorPosCallback*>(event.event_data);
                Agos::AgGLFWHandlerCursorPosEventHandler::process(*event_data, m_RendererReference);
            }
            break;
        }
        case Agos::Events::keyboardCallback:
        {
            Agos::Events::AgGLFWEventKeyboardCallback* event_data = reinterpret_cast<Agos::Events::AgGLFWEventKeyboardCallback*>(event.event_data);
            Agos::AgGLFWHandlerKeyboardEventHandler::process(*event_data, this, m_RendererReference);
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
    AgGLFWHandlerInstance* glfw_instance,
    AgVulkanHandlerRenderer* renderer)
{
    switch (event_data.key)
    {
    case GLFW_KEY_W:
    {
        renderer->m_Camera->compute_camera_basis();
        renderer->m_Camera->m_CameraPosition += renderer->m_Camera->m_CameraSpeed * (-renderer->m_Camera->m_CameraOppositeDirection);
        break;
    }
    case GLFW_KEY_S:
    {
        renderer->m_Camera->compute_camera_basis();
        renderer->m_Camera->m_CameraPosition -= renderer->m_Camera->m_CameraSpeed * (-renderer->m_Camera->m_CameraOppositeDirection);
        break;
    }
    case GLFW_KEY_D:
    {
        renderer->m_Camera->compute_camera_basis();
        renderer->m_Camera->m_CameraPosition += renderer->m_Camera->m_CameraRight * renderer->m_Camera->m_CameraSpeed;
        break;
    }
    case GLFW_KEY_A:
    {
        renderer->m_Camera->compute_camera_basis();
        renderer->m_Camera->m_CameraPosition -= renderer->m_Camera->m_CameraRight * renderer->m_Camera->m_CameraSpeed;
        break;
    }
    case GLFW_KEY_Z:
    {
        if (glfw_instance->get_cursor_state() == GLFW_CURSOR_DISABLED)
        {
            glfwSetInputMode(glfw_instance->get_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfw_instance->get_cursor_state() = GLFW_CURSOR_NORMAL;
        }
        else if (glfw_instance->get_cursor_state() == GLFW_CURSOR_NORMAL)
        {
            glfwSetInputMode(glfw_instance->get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfw_instance->get_cursor_state() = GLFW_CURSOR_DISABLED;
        }
        break;
    }
    
    default:
        AG_CORE_INFO("[GLFW/AgGLFWHandlerKeyboardEventHandler - process] Unkown key pressed!");
        break;
    }
}


bool Agos::AgGLFWHandlerCursorPosEventHandler::firstMouse = true;
float Agos::AgGLFWHandlerCursorPosEventHandler::lastX = 0.0f;
float Agos::AgGLFWHandlerCursorPosEventHandler::lastY = 0.0f;

void Agos::AgGLFWHandlerCursorPosEventHandler::process(
    const Agos::Events::AgGLFWEventCursorPosCallback& event,
    AgVulkanHandlerRenderer* renderer
)
{
    if (firstMouse)
    {
        lastX = event.xpos;
        lastY = event.ypos;
        firstMouse = false;
        return;
    }
  
    float xoffset = event.xpos - lastX;
    float yoffset = lastY - event.ypos; 
    lastX = event.xpos;
    lastY = event.ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    renderer->m_Camera->m_CameraYaw   += xoffset;
    renderer->m_Camera->m_CameraPitch += yoffset;

    if(renderer->m_Camera->m_CameraPitch > 89.0f)
        renderer->m_Camera->m_CameraPitch = 89.0f;
    if(renderer->m_Camera->m_CameraPitch < -89.0f)
        renderer->m_Camera->m_CameraPitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(renderer->m_Camera->m_CameraYaw)) * cos(glm::radians(renderer->m_Camera->m_CameraPitch));
    direction.y = sin(glm::radians(renderer->m_Camera->m_CameraPitch));
    direction.z = sin(glm::radians(renderer->m_Camera->m_CameraYaw)) * cos(glm::radians(renderer->m_Camera->m_CameraPitch));
    renderer->m_Camera->m_CameraOppositeDirection = std::move(glm::normalize(direction * (-1.0f) ));
}
