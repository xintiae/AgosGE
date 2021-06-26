#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include AG_EVENTBUS_INCLUDE
#include AG_GLFW_INCLUDE
#include <memory>

namespace Agos
{
namespace Events
{
// see GLFW documentation on why and how buttons and actions work
typedef int AgGLFWHandlerMouseButton;
typedef int AgGLFWHandlerMouseAction;

typedef enum AgGLFWHandlerEventType : uint8_t {
    undefined                   = 0,
    framebufferResizeCallback   = 1,
    cursorPosCallback           = 2,
    mouseButtonCallback         = 3
} AgGLFWHandlerEventType;

typedef struct AG_API AgGLFWEventFramebufferResizeCallback {
    GLFWwindow* window;
    int new_width;
    int new_height;
} AgGLFWHandlerEventFramebufferResizeCallback;

typedef struct AG_API AgGLFWEventCursorPosCallback {
    GLFWwindow* window;
    double xpos;
    double ypos;
} AgGLFWHandlerEventCursorPosCallback;

typedef struct AG_API AgGLFWEventMouseButtonCallback {
    GLFWwindow* window;
    AgGLFWHandlerMouseButton button;
    AgGLFWHandlerMouseAction action;
} AgGLFWHandlerEventMouseButtonCallback;

typedef struct AG_API AgGLFWHandlerEvent
{
    AgGLFWHandlerEventType type = AgGLFWHandlerEventType::undefined;
    void* event_data;
} AgGLFWHandlerEvent;
}   // namespace Events (within namespace Agos)


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
