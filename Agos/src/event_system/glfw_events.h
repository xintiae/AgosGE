#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include <Agos/vendor/EventBus/lib/src/dexode/EventBus.hpp>
#include <GLFW/glfw3.h>
#include <memory>

namespace Agos
{
// see GLFW documentation on why and how buttons and actions work
typedef int AgGLFWHandlerMouseButton;
typedef int AgGLFWHandlerMouseAction;

typedef struct AG_API AgGLFWHandlerEventTypes
{
    enum type : uint8_t
    {
        undefined                   = 0,
        framebufferResizeCallback   = 1,
        cursorPosCallback           = 2,
        mouseButtonCallback         = 3
    };

    struct glfwFramebufferResizeCallback
    {
        type t  = type::undefined;
        GLFWwindow* window;
        int new_width;
        int new_height;
    };
    struct glfwCursorPosCallback
    {
        type t  = type::undefined;
        GLFWwindow* window;
        double xpos;
        double ypos;
    };
    struct tglfwMouseButtonCallback
    {
        type t  = type::undefined;
        GLFWwindow* window;
        AgGLFWHandlerMouseButton button;
        AgGLFWHandlerMouseAction action;
    };
} AgGLFWHandlerEventTypes;

typedef class AG_API AgGLFWHandlerEvents
{
private:
    static std::shared_ptr<dexode::EventBus> m_EventBus;

public:
    AgGLFWHandlerEvents(const std::shared_ptr<dexode::EventBus>& event_bus);
    ~AgGLFWHandlerEvents();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
} AgGLFWHandlerEvents;

} // namespace Agos
