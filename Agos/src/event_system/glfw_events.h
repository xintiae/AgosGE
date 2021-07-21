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

enum AgGLFWHandlerEventType : uint8_t {
    undefined                   = 0,
    framebufferResizeCallback   = 1,
    cursorPosCallback           = 2,
    mouseButtonCallback         = 3,
    keyboardCallback            = 4
};  // enum AgGLFWHandlerEventType

struct AG_API AgGLFWEventFramebufferResizeCallback {
    GLFWwindow* window;
    int new_width;
    int new_height;
};  // struct AgGLFWHandlerEventFramebufferResizeCallback

struct AG_API AgGLFWEventCursorPosCallback {
    GLFWwindow* window;
    double xpos;
    double ypos;
};  // struct AgGLFWHandlerEventCursorPosCallback;

struct AG_API AgGLFWEventMouseButtonCallback {
    GLFWwindow* window;
    AgGLFWHandlerMouseButton button;
    AgGLFWHandlerMouseAction action;
};  // struct AgGLFWHandlerEventMouseButtonCallback;

struct AG_API AgGLFWEventKeyboardCallback {
    GLFWwindow* window; 
    int key;
    int scancode;
    int action;
    int mods;
};  // struct AgGLFWEventKeyboardCallback


struct AG_API AgGLFWHandlerEvent
{
    AgGLFWHandlerEventType type = AgGLFWHandlerEventType::undefined;
    void* event_data;
};  // struct AgGLFWHandlerEvent
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
    static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
} AgGLFWHandlerEvents;

} // namespace Agos
