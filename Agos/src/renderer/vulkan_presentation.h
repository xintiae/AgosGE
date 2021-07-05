#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"
#include "Agos/src/renderer/vulkan_buffers.h"

#include AG_VULKAN_INCLUDE
#include <cstdint>
#include <memory>

namespace Agos
{
typedef class AG_API AgVulkanHandlerPresenter
{
private:
    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;
    std::vector<VkFence> m_ImagesInFlight;
    size_t m_CurrentFrame;

    VkDevice m_LogicalDeviceReference;
    bool m_SemaphoresFencesTerminated = false;
    bool m_Terminated = false;

public:
    AgVulkanHandlerPresenter();
    AgVulkanHandlerPresenter(const VkDevice& logical_device);
    ~AgVulkanHandlerPresenter();

    AgResult create_semaphores_fences_objs(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain);
    AgResult draw_frame(        // oh boy! the one we were waiting for
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::shared_ptr<AgVulkanHandlerBufferManager>& command_bufffers);
    AgResult terminate_semaphores_fences_objs();

    AgResult terminate();

protected:
    static void update_uniform_buffer(
        const uint32_t& current_image,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::shared_ptr<AgVulkanHandlerBufferManager>& uniform_buffers);
        
} AgVulkanHandlerPresenter;
} // namespace Agos


