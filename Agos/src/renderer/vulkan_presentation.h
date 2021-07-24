#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"
namespace Agos{
    class AgVulkanHandlerVIUBufferManager;
    class AgVulkanHandlerCommandBufferManager;
}
#include "Agos/src/renderer/vulkan_buffers.h"
#include "Agos/src/renderer/model.h"

#include AG_VULKAN_INCLUDE
#include <cstdint>
#include <memory>

namespace Agos
{
class AG_API AgVulkanHandlerPresenter
{
private:
    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;
    std::vector<VkFence> m_ImagesInFlight;
    size_t m_CurrentFrame;

    VkDevice& m_LogicalDeviceReference;
    bool m_SemaphoresFencesTerminated = false;
    bool m_Terminated = false;

public:
    AgVulkanHandlerPresenter();
    AgVulkanHandlerPresenter(VkDevice& logical_device);
    ~AgVulkanHandlerPresenter();

    AgResult create_semaphores_fences_objs(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain);
    AgResult draw_frame(        // oh boy! the one we were waiting for
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::vector<std::shared_ptr<AgVulkanHandlerVIUBufferManager>>& uniform_command_bufffers,
        const std::vector<AgModel>& models,
        const std::shared_ptr<AgVulkanHandlerCommandBufferManager>& command_buffers_manager,
        AgVulkanHandlerRenderer* renderer);
    AgResult terminate_semaphores_fences_objs();

    AgResult terminate();

    std::vector<VkSemaphore>& get_image_available_semaphores();
    std::vector<VkSemaphore>& get_render_finished_semaphores();
    std::vector<VkFence>& get_in_flight_fences();
    std::vector<VkFence>& get_images_in_flight();

protected:
    static void update_uniform_buffer(
        const uint32_t& current_image,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::vector<std::shared_ptr<AgVulkanHandlerVIUBufferManager>>& uniforms_buffers,
        const glm::vec3& camera_position,
        const glm::vec3& camera_target,
        const glm::vec3& camera_orientation,
        const std::vector<AgModel>& models);
};  // class AgVulkanHandlerPresenter

} // namespace Agos
