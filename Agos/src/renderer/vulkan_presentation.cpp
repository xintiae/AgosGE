#include "Agos/src/renderer/vulkan_presentation.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/vulkan_descriptor.h"
#include AG_GLM_INCLUDE
#include <chrono>

extern VkDevice        AG_DEFAULT_LOGICAL_DEVICE_REFERENCE;


Agos::AgVulkanHandlerPresenter::AgVulkanHandlerPresenter()
    : m_LogicalDeviceReference(AG_DEFAULT_LOGICAL_DEVICE_REFERENCE), m_CurrentFrame(0)
{
}

Agos::AgVulkanHandlerPresenter::AgVulkanHandlerPresenter(VkDevice& logical_device)
    : m_LogicalDeviceReference(logical_device), m_CurrentFrame(0)
{
}

Agos::AgVulkanHandlerPresenter::~AgVulkanHandlerPresenter()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerPresenter::create_semaphores_fences_objs(
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain
)
{
    m_LogicalDeviceReference = logical_device->get_device();

    m_ImageAvailableSemaphores.resize(AG_VULKAN_MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(AG_VULKAN_MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(AG_VULKAN_MAX_FRAMES_IN_FLIGHT);
    m_ImagesInFlight.resize(swapchain->get_swapchain_images().size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < AG_VULKAN_MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (    vkCreateSemaphore(logical_device->get_device(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i])  != VK_SUCCESS ||
                vkCreateSemaphore(logical_device->get_device(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i])  != VK_SUCCESS ||
                vkCreateFence(logical_device->get_device(), &fenceInfo, nullptr, &m_InFlightFences[i])                    != VK_SUCCESS)
        {
            throw std::runtime_error("[Vulkan/AgVulkanHandlerPresenter - create_semaphores_fences_objs] Failed to create synchronization objects for a frame!");
        }
    }

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerPresenter - create_semaphores_fences_objs] Created semaphores and fences objs!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerPresenter::draw_frame(
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
    const std::vector<std::shared_ptr<AgVulkanHandlerVIUBufferManager>>& uniform_command_bufffers,
    const std::shared_ptr<AgVulkanHandlerCommandBufferManager>& command_buffers_manager,
    AgVulkanHandlerRenderer* renderer
)
{
    vkWaitForFences(logical_device->get_device(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        logical_device->get_device(),
        swapchain->get_swapchain(),
        UINT64_MAX,
        m_ImageAvailableSemaphores[m_CurrentFrame],
        VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        renderer->recreate_swapchain(false);
        return AG_RECREATED_SWAPCHAIN;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerPresenter - draw_frame] Failed to acquire swap chain image!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerPresenter - draw_frame] Failed to acquire swap chain image!");
    }

    for (size_t i = 0; i < uniform_command_bufffers.size(); i++)
    {
        update_uniform_buffer(
            imageIndex,
            logical_device,
            swapchain,
            uniform_command_bufffers[i],
            renderer->m_Camera->m_CameraPosition,
            renderer->m_Camera->m_CameraTarget,
            // renderer->m_Camera->m_CameraUp,
            renderer->m_Camera->m_Up,
            glm::vec3(5.0f));
    }

    if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(logical_device->get_device(), 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];


    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;

    VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffers_manager->get_command_buffers()[imageIndex];

    vkResetFences(logical_device->get_device(), 1, &m_InFlightFences[m_CurrentFrame]);
    if (vkQueueSubmit(logical_device->get_graphics_queue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerPresenter - draw_frame] Failed to submit draw command buffer!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerPresenter - draw_frame] Failed to submit draw command buffer!");
    }


    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapchain->get_swapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(logical_device->get_present_queue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || renderer->m_FramebufferResized)
    {
        renderer->m_FramebufferResized = false;
        renderer->recreate_swapchain(false);
    }
    else if (result != VK_SUCCESS)
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("[Vulkan/AgVulkanHandlerPresenter - draw_frame] Failed to present swap chain image!");
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % AG_VULKAN_MAX_FRAMES_IN_FLIGHT;
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerPresenter::terminate_semaphores_fences_objs()
{
    if (!m_SemaphoresFencesTerminated)
    {
        for (size_t i = 0; i < AG_VULKAN_MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_LogicalDeviceReference, m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_LogicalDeviceReference, m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_LogicalDeviceReference, m_InFlightFences[i], nullptr);
        }
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerPresenter - terminate_semaphores_fences_objs] Destroyed semaphores, destroyed fences!");
        m_SemaphoresFencesTerminated = true;
        return AG_SUCCESS;
    }
    return AG_SEMAPHORES_FENCES_ALREADY_TERMINATED;
}

Agos::AgResult Agos::AgVulkanHandlerPresenter::terminate()
{
    if (!m_Terminated)
    {
        terminate_semaphores_fences_objs();
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerPresenter - terminate] Terminated instance!");
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

std::vector<VkSemaphore>& Agos::AgVulkanHandlerPresenter::get_image_available_semaphores()
{
    return m_ImageAvailableSemaphores;
}

std::vector<VkSemaphore>& Agos::AgVulkanHandlerPresenter::get_render_finished_semaphores()
{
    return m_RenderFinishedSemaphores;
}

std::vector<VkFence>& Agos::AgVulkanHandlerPresenter::get_in_flight_fences()
{
    return m_InFlightFences;
}

std::vector<VkFence>& Agos::AgVulkanHandlerPresenter::get_images_in_flight()
{
    return m_ImagesInFlight;
}

void Agos::AgVulkanHandlerPresenter::update_uniform_buffer(
    const uint32_t& current_image,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
    const std::shared_ptr<AgVulkanHandlerVIUBufferManager>& uniform_buffers,
    const glm::vec3& camera_position,
    const glm::vec3& camera_target,
    const glm::vec3& camera_orientation,
    const glm::vec3& light_position
)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    Agos::VulkanGraphicsPipeline::UniformBufferObject ubo{};

    // ubo.model = glm::rotate( glm::mat4(1.0f), time * glm::radians(90.0f), glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)) );
    ubo.model = glm::mat4(1.0f);
    ubo.view = glm::lookAt(camera_position, camera_target, camera_orientation);
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchain->get_swapchain_extent().width / (float)swapchain->get_swapchain_extent().height, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;

    ubo.lightPos = light_position;

    void *data;
    vkMapMemory(logical_device->get_device(), uniform_buffers->get_uniform_buffers_memory()[current_image], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(logical_device->get_device(), uniform_buffers->get_uniform_buffers_memory()[current_image]);
}
