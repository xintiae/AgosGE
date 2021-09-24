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

namespace Event
{

// ** - - - - - - - - - - - - - - - - - - - Event types - - - - - - - - - - - - - - - - - - -

enum Type : uint8_t
{
    undefined                   = 0,
    framebufferResizeCallback   = 1,
    cursorPosCallback           = 2,
    mouseButtonCallback         = 3,
    keyboardCallback            = 4
};

// ** - - - - - - - - - - - - - - - - - - - Event types - - - - - - - - - - - - - - - - - - -


// ** - - - - - - - - - - - - - - - - - - - Event manager - - - - - - - - - - - - - - - - - - -

class AG_API EventManager
{
private:
    static std::shared_ptr<dexode::EventBus> m_EventBus;

public:
    EventManager();
    EventManager(const std::shared_ptr<dexode::EventBus>& event_bus);
    ~EventManager();

    static std::shared_ptr<dexode::EventBus>&       get_event_bus();

    static void     framebufferResizeCallback       (GLFWwindow* window, int width, int height);
    static void     cursorPositionCallback          (GLFWwindow* window, double xpos, double ypos);
    static void     mouseButtonCallback             (GLFWwindow* window, int button, int action, int mods);
    static void     keyboardCallback                (GLFWwindow* window, int key, int scancode, int action, int mods);

};
// ** - - - - - - - - - - - - - - - - - - - Event manager - - - - - - - - - - - - - - - - - - -


// ** - - - - - - - - - - - - - - - - - - - Event struct - - - - - - - - - - - - - - - - - - -

// struct Event contains two members : a pointer to the event's data, which is determined by the type of event that is currently being raised
struct AG_API Event
{
    Agos::GLFWHandler::Event::Type type = Type::undefined;
    void* event_data;
};

// ** - - - - - - - - - - - - - - - - - - - Event struct - - - - - - - - - - - - - - - - - - -


// ** - - - - - - - - - - - - - - - - - - - Event callbacks - - - - - - - - - - - - - - - - - - -

// * namespace Callbacks
// consists of a bunch of structs that describes what kind of information is raised when triggering specific events
namespace Callbacks
{

// framebuffer's been resized
struct AG_API FramebufferResize {
    GLFWwindow* window;
    int new_width;
    int new_height;
};

// user moved his mouse
struct AG_API CursorPosition {
    GLFWwindow* window;
    double xpos;
    double ypos;
};

// user pressed a button on his mouse (right click...)
struct AG_API MouseButton {
    GLFWwindow* window;
    int button;
    int action;
};

struct AG_API Keyboard {
    GLFWwindow* window; 
    int key;
    int scancode;
    int action;
    int mods;
};

}   // * namespace Callbacks (within namespace Agos::GLFWHandler::Event)

// ** - - - - - - - - - - - - - - - - - - - Event callbacks - - - - - - - - - - - - - - - - - - -


}   // namespace Event (within namespace GLFWHandler, itself within namespace Agos)

}   // namespace GLFWHandler (within namespace Agos)

}   // namespace Agos
