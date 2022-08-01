#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"
#include "Agos/src/event_system/glfw_events.h"


#include AG_EVENTBUS_INCLUDE
#include AG_GLFW_INCLUDE
#include AG_VULKAN_INCLUDE
#include <memory>

namespace Agos
{

namespace GLFWHandler
{

class AG_API GLFWInstance
{
private:
    dexode::EventBus::Listener  m_EventBusListener;
    GLFWwindow*                 m_ApplicationWindow;
    size_t                      m_CursorState;          // = 0;
    bool                        m_Terminated;           // = false;

public:
    // include renderer for GLFWInstance to inform about mouse pos, framebuffer new size, etc...
    GLFWInstance(const std::shared_ptr<dexode::EventBus>& event_bus);
    GLFWInstance(const GLFWInstance& other)             = delete;
    GLFWInstance(GLFWInstance&& other)                  = delete;
    ~GLFWInstance();

    GLFWInstance& operator=(const GLFWInstance& other)  = delete;
    GLFWInstance& operator=(GLFWInstance& other)        = delete;


    AgResult    init       (const std::shared_ptr<Agos::GLFWHandler::Event::EventManager>& event_manager,
                            const std::string& window_title = "AgosGE",
                            const int&          width   = AG_DEFAULT_WINDOW_WIDTH,
                            const int&          height  = AG_DEFAULT_WINDOW_HEIGHT,
                            const               bool& shall_cursor_exist = false);
    AgBool      app_should_run();
    AgResult    terminate  ();

    // misc
    GLFWwindow*&    get_window      ();
    size_t&         get_cursor_state();

private:
    void on_event_process(const Agos::GLFWHandler::Event::Event& event);

};  // class GLFWInstance

namespace EventProcessor
{
// helper struct to handle from keyboard triggered events
struct AG_API Keyboard
{
    static void process(
        const Agos::GLFWHandler::Event::Callbacks::Keyboard& event_data,
        GLFWHandler::GLFWInstance* glfw_instance
    );
};

// helper struct to handle from mouse (and specificaly its position) triggered events
struct AG_API CursorPosition
{
    static bool     firstMouse;
    static float    lastX, lastY;

    static void process(const Agos::GLFWHandler::Event::Callbacks::CursorPosition& event);
};

}   // namespace EventProcessor (within namespace GLFWHandler, itself within namespace Agos)

}   // namespace GLFWHandler (within namespace Agos)

} // namespace Agos
