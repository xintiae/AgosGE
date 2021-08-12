#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/renderer/imgui/mandatories/imgui.h"
#include "Agos/src/renderer/glfw_instance.h"
#include "Agos/src/renderer/vulkan_instance.h"
namespace Agos{
    struct AgQueueFamilyIndices;
    class AgVulkanHandlerPhysicalDevice;
}
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"
#include "Agos/src/renderer/vulkan_render_pass.h"
#include "Agos/src/renderer/renderer.h"
namespace Agos{
    class AgVulkanHandlerCommandPoolManager;
}
#include "Agos/src/renderer/vulkan_command_pool.h"


#include "Agos/src/renderer/imgui/mandatories/imgui_impl_vulkan.h"
#include "Agos/src/renderer/imgui/mandatories/imgui_impl_glfw.h"

#include AG_GLFW_INCLUDE
#include <memory>


namespace Agos
{
class AG_API AgImGuiHandler
{
private:
    VkDescriptorPool m_ImguiDescriptorPool;

    bool m_ImguiDescriptorPoolTerminated    = false;

    VkDevice& m_LogicalDeviceReference;
    GLFWwindow*& m_ApplicationWindowReference;

    bool m_ImguiTerminated = false;

public:
    AgImGuiHandler(GLFWwindow*& application_window);
    ~AgImGuiHandler();

    AgResult init(
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance,
        const std::shared_ptr<AgVulkanHandlerInstance>& vulkan_instance,
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::shared_ptr<AgVulkanHandlerRenderPass>& renderpass,
        const std::shared_ptr<AgVulkanHandlerCommandPoolManager>& command_pool_manager);

    AgResult update_ui();
    AgResult terminate(const bool& mark_as_terminated = false);

private:
    AgResult create_descriptor_pool(
        const VkDevice& logical_device);
    AgResult terminate_descriptor_pool();

    void create_context();

    void upload_front_textures(
        const VkDevice& logical_device,
        const VkQueue& graphics_queue,
        const VkCommandPool& command_pool
    );
};

static void ag_imgui_check_vk_result(VkResult err);
}
