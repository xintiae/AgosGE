#include "Agos/src/renderer/glfw/glfw_instance.h"

#include "Agos/src/logger/logger.h"
#include AG_GLM_INCLUDE


Agos::GLFWHandler::GLFWInstance::GLFWInstance()
{
    m_ApplicationEventBus       = std::make_shared<dexode::EventBus>();
    m_ApplicationEventManager   = std::make_shared<Agos::GLFWHandler::GLFWEvent::EventManager>(m_ApplicationEventBus);
    m_EventBusListener          = std::make_shared<dexode::EventBus::Listener>(m_ApplicationEventBus);
}

Agos::GLFWHandler::GLFWInstance::~GLFWInstance()
{
    terminate();
}

Agos::AgResult Agos::GLFWHandler::GLFWInstance::init(
    const std::string&  window_title /*= "AgosGE"*/,
    const int&          width /*= AG_DEFAULT_WINDOW_WIDTH*/,
    const int&          height /*= AG_DEFAULT_WINDOW_HEIGHT*/,
    const bool&         shall_cursor_exist /*= false*/
)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_ApplicationWindow = NULL;
    if ( (m_ApplicationWindow = glfwCreateWindow(width, height , window_title.c_str(), nullptr, nullptr)) == NULL)
    {
        AG_CORE_CRITICAL("[GLFW/GLFWHandler::GLFWInstance - init] Failed to create window!");
        throw std::runtime_error("[GLFW/GLFWHandler::GLFWInstance - init] Failed to create window!");
        return AG_FAILED_TO_CREATE_GLFW_INSTANCE;
    }

    glfwSetWindowUserPointer        (m_ApplicationWindow, this);
    glfwSetInputMode                (m_ApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetFramebufferSizeCallback  (m_ApplicationWindow, m_ApplicationEventManager->framebufferResizeCallback);

    return AG_SUCCESS;
}

Agos::AgBool Agos::GLFWHandler::GLFWInstance::app_should_run()
{
    glfwPollEvents();
    return !glfwWindowShouldClose(m_ApplicationWindow);
}

Agos::AgResult Agos::GLFWHandler::GLFWInstance::terminate()
{
    if (!m_Terminated)
    {
        m_ApplicationEventManager->~EventManager();

        glfwDestroyWindow(m_ApplicationWindow);
        glfwTerminate();

        AG_CORE_INFO("[GLFW/Agos::GLFWHandler::GLFWInstance - terminate] Terminated GLFW instance!");
        m_Terminated = true;        
        return AG_SUCCESS;
    }

    return AG_INSTANCE_ALREADY_TERMINATED;
}

/*
void Agos::GLFWHandler::GLFWInstance::on_event_process(const Agos::GLFWHandler::GLFWEvent::Event& event)
{
    switch (event.type)
    {
        case Agos::GLFWHandler::GLFWEvent::Type::framebufferResizeCallback:
        {
            // m_RendererReference->m_FramebufferResized = true;
            // m_RendererReference->recreate_swapchain(false);
            break;
        }
        case Agos::GLFWHandler::GLFWEvent::Type::mouseButtonCallback:
        {
            // clicky stuff goes brrrrrrrrrr
            break;
        }
        case Agos::GLFWHandler::GLFWEvent::Type::cursorPosCallback:
        {
            if (this->m_CursorState == GLFW_CURSOR_DISABLED)
            {
                Agos::GLFWHandler::GLFWEvent::Callbacks::CursorPosition* event_data = reinterpret_cast<Agos::GLFWHandler::GLFWEvent::Callbacks::CursorPosition*>(event.event_data);
                Agos::GLFWHandler::EventProcessor::CursorPosition::process(*event_data); //, m_RendererReference);
            }
            break;
        }
        case Agos::GLFWHandler::GLFWEvent::Type::keyboardCallback:
        {
            Agos::GLFWHandler::GLFWEvent::Callbacks::Keyboard* event_data = reinterpret_cast<Agos::GLFWHandler::GLFWEvent::Callbacks::Keyboard*>(event.event_data);
            Agos::GLFWHandler::EventProcessor::Keyboard::process(*event_data, this); //, m_RendererReference);
            break;
        }
        case Agos::GLFWHandler::GLFWEvent::Type::undefined:
        {
            AG_CORE_WARN("[GLFW/AgGLFWHandlerInstance - on_event_process] Tryied to process undefined event!");
            break;
        }
    }
}

void Agos::GLFWHandler::EventProcessor::Keyboard::process(
    const Agos::GLFWHandler::GLFWEvent::Callbacks::Keyboard& event_data,
    Agos::GLFWHandler::GLFWInstance* glfw_instance)
{
    switch (event_data.key)
    {
    case GLFW_KEY_W:
    {
        // renderer->m_Camera->compute_camera_basis();
        // renderer->m_Camera->m_CameraPosition += renderer->m_Camera->m_CameraSpeed * (-renderer->m_Camera->m_CameraOppositeDirection);
        break;
    }
    case GLFW_KEY_S:
    {
        // renderer->m_Camera->compute_camera_basis();
        // renderer->m_Camera->m_CameraPosition -= renderer->m_Camera->m_CameraSpeed * (-renderer->m_Camera->m_CameraOppositeDirection);
        break;
    }
    case GLFW_KEY_D:
    {
        // renderer->m_Camera->compute_camera_basis();
        // renderer->m_Camera->m_CameraPosition += renderer->m_Camera->m_CameraRight * renderer->m_Camera->m_CameraSpeed;
        break;
    }
    case GLFW_KEY_A:
    {
        // renderer->m_Camera->compute_camera_basis();
        // renderer->m_Camera->m_CameraPosition -= renderer->m_Camera->m_CameraRight * renderer->m_Camera->m_CameraSpeed;
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
        AG_CORE_INFO("[GLFW/AgGLFWHandlerKeyboardEventProcessor - process] Unkown key pressed!");
        break;
    }
}


bool Agos::GLFWHandler::EventProcessor::CursorPosition::firstMouse = true;
float Agos::GLFWHandler::EventProcessor::CursorPosition::lastX = 0.0f;
float Agos::GLFWHandler::EventProcessor::CursorPosition::lastY = 0.0f;

void Agos::GLFWHandler::EventProcessor::CursorPosition::process(
    const Agos::GLFWHandler::GLFWEvent::Callbacks::CursorPosition& event)
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

    // renderer->m_Camera->m_CameraYaw   += xoffset;
    // renderer->m_Camera->m_CameraPitch += yoffset;

    // if(renderer->m_Camera->m_CameraPitch > 89.0f)
        // renderer->m_Camera->m_CameraPitch = 89.0f;
    // if(renderer->m_Camera->m_CameraPitch < -89.0f)
        // renderer->m_Camera->m_CameraPitch = -89.0f;

    glm::vec3 direction;
    AG_MARK_AS_USED(direction);
    // direction.x = cos(glm::radians(renderer->m_Camera->m_CameraYaw)) * cos(glm::radians(renderer->m_Camera->m_CameraPitch));
    // direction.y = sin(glm::radians(renderer->m_Camera->m_CameraPitch));
    // direction.z = sin(glm::radians(renderer->m_Camera->m_CameraYaw)) * cos(glm::radians(renderer->m_Camera->m_CameraPitch));
    // renderer->m_Camera->m_CameraOppositeDirection = std::move(glm::normalize(direction * (-1.0f) ));
}
*/
