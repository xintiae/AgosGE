#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/imgui/backends/imgui.h"
#include "Agos/src/renderer/imgui/backends/imgui_implementations/imgui_impl_glfw.h"
#include "Agos/src/renderer/imgui/backends/imgui_implementations/imgui_impl_vulkan.h"
#include "Agos/src/renderer/glfw_instance.h"
#include AG_VULKAN_INCLUDE


namespace Agos
{
namespace ImGuiHandler
{
namespace ImGuiVulkan
{

struct AG_API ImGui_initInfo
{
    GLFWwindow*                     Window;
    VkInstance                      Instance;
    VkPhysicalDevice                PhysicalDevice;
    VkDevice*                       Device;
    uint32_t                        QueueFamily;
    VkQueue                         Queue;
    VkPipelineCache                 PipelineCache;
    VkRenderPass                    RenderPass;
    // VkDescriptorPool                DescriptorPool;
    VkCommandPool                   CommandPool;
    uint32_t                        Subpass;
    uint32_t                        MinImageCount;          // >= 2
    uint32_t                        ImageCount;             // >= MinImageCount
    VkSampleCountFlagBits           MSAASamples;            // >= VK_SAMPLE_COUNT_1_BIT
    const VkAllocationCallbacks*    Allocator;
    void                            (*CheckVkResultFn)(VkResult err);
};

class AG_API ImGuiInstance
{
private:
    VkDevice*                       m_LogicalDeviceRef;
    const VkAllocationCallbacks*    m_AllocatorRef;
    VkDescriptorPool                m_DescriptorPool;
    std::string                     m_IniPath;          // = AG_SOURCE_PATH + AG_IMGUI_INI_FILE_PATH

    bool                            m_ImGuiTerminated;   // = false

public:
    ImGuiInstance();
    ImGuiInstance(const ImGuiInstance& other)   = delete;
    ImGuiInstance(ImGuiInstance&& other)        = delete;
    ~ImGuiInstance();

    ImGuiInstance& operator=(const ImGuiInstance& other)    = delete;
    ImGuiInstance& operator=(ImGuiInstance&& other)         = delete;


    AgResult    init        (Agos::ImGuiHandler::ImGuiVulkan::ImGui_initInfo* init_info);
    AgResult    new_frame   ();
    AgResult    end_frame   ();
    AgResult    draw_frame  (const VkCommandBuffer& command_buffer);
    AgResult    terminate   ();

private:
    AgResult    create_descriptor_pool();
    AgResult    destroy_descriptor_pool();

};  // * class ImGui (within namespace Agos::VulkanHandler)
static inline void AG_API ag_vulkan_imgui_check_err(VkResult err)
{
    if (err == VK_SUCCESS)
        return;
    else
    {
        AG_CORE_ERROR("[Vulkan - ImGui/Agos::VulkanHandler::ag_vulkan_imgui_check_err] Error triggered! Error code : " + std::to_string(err));
    }
}

}   // namespace ImGuiVulkan (within namespace Agos::ImGuiHandler)
}   // namespace ImGuiHandler (within namespace Agos)
}   // namespace Agos
