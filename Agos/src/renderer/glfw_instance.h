#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"
#include "Agos/src/renderer/vulkan_instance.h"
#include "Agos/src/event_system/glfw_events.h"

#include <Agos/vendor/EventBus/lib/src/dexode/EventBus.hpp>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <memory>

namespace Agos
{
typedef class AG_API AgGLFWHandlerInstance
{
private:
    dexode::EventBus::Listener m_EventBusListener;
    GLFWwindow* m_ApplicationWindow;
    VkSurfaceKHR m_ApplicationSurface;

    bool m_Terminated = false;

public:
    AgGLFWHandlerInstance(const std::shared_ptr<dexode::EventBus>& event_bus);
    ~AgGLFWHandlerInstance();

    AgGLFWHandlerInstance(const AgGLFWHandlerInstance& other)   = delete;
    AgGLFWHandlerInstance(AgGLFWHandlerInstance&& other)        = delete;

    AgResult init(const std::shared_ptr<AgGLFWHandlerEvents>& event_handler);
    AgResult setup_vulkan_surface(const std::shared_ptr<AgVulkanHandlerInstance>& vulkan_instance);
    AgResult terminate_vulkan_surface(const std::shared_ptr<AgVulkanHandlerInstance>& vulkan_instance);
    AgResult terminate();

    GLFWwindow*& get_window();
    VkSurfaceKHR& get_surface();
private:
    void on_event_process(const Agos::Events::AgGLFWHandlerEvent& event);
} AgGLFWHandlerInstance;

} // namespace Agos
