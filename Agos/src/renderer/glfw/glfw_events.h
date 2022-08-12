#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include AG_EVENTBUS_INCLUDE
#include AG_GLFW_INCLUDE
#include <memory>

namespace Agos
{
namespace GLFWHandler
{
namespace GLFWEvent
{

enum EventType : int8_t
{
    invalid                     = -1,
    undefined                   =  0,
    framebufferResizeCallback   =  1,
    cursorPosCallback           =  2,
    mouseButtonCallback         =  3,
    keyboardCallback            =  4
};

namespace EventData
{
    struct FramebufferResize
    {
        GLFWwindow* window;
        int new_width;
        int new_height;
    };
    
    struct CursorPosition
    {
        GLFWwindow* window;
        double xpos;
        double ypos;
    };
    
    struct MouseButton
    {
        GLFWwindow* window;
        int button;
        int action;
    };
    
    struct Keyboard
    {
        GLFWwindow* window; 
        int key;
        int scancode;
        int action;
        int mods;
    };
}   // * namespace EventData (within namespace Agos::GLFWHandler::GLFWEvent)


// ? using glfwGetXXX, do we really need void* data?
// ? is it faster to glfwGetXXX than allocating new mem every time?
struct Event
{
    EventType type = EventType::undefined;
    // void* data;
};


class EventManager
{
private:
    // static ref to the application event bus (see @file Agos/src/renderer/renderer.h)
    static std::shared_ptr<dexode::EventBus>    m_EventBusRef;

public:
    EventManager(const std::shared_ptr<dexode::EventBus>& event_bus);
    ~EventManager();

    EventManager(const EventManager& other) = delete;
    EventManager(EventManager&& other)      = delete;

    EventManager& operator=(const EventManager& other)  = delete;
    EventManager& operator=(EventManager&& other)       = delete;

    static inline std::shared_ptr<dexode::EventBus>&    get_event_bus()     { return m_EventBusRef; }

    static void     framebufferResizeCallback       (GLFWwindow* window, int width,     int height                          );
    static void     cursorPositionCallback          (GLFWwindow* window, double xpos,   double ypos                         );
    static void     mouseButtonCallback             (GLFWwindow* window, int button,    int action,     int mods            );
    static void     keyboardCallback                (GLFWwindow* window, int key,       int scancode,   int action, int mods);
};

}   // ** namespace Event (within namespace GLFWHandler::Agos)
}   // * namespace GLFWHandler (within namespace Agos)
}   // * namespace Agos
