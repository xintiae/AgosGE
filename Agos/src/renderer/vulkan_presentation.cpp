#include "Agos/src/renderer/vulkan_presentation.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/vulkan_descriptor.h"

#include AG_GLM_INCLUDE

extern VkDevice        AG_DEFAULT_LOGICAL_DEVICE_REFERENCE;


Agos::AgVulkanHandlerPresenter::AgVulkanHandlerPresenter()
    : m_CurrentFrame(0), m_LogicalDeviceReference(AG_DEFAULT_LOGICAL_DEVICE_REFERENCE)
{
}

Agos::AgVulkanHandlerPresenter::AgVulkanHandlerPresenter(VkDevice& logical_device)
    : m_CurrentFrame(0), m_LogicalDeviceReference(logical_device)
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
    const std::vector<std::shared_ptr<AgVulkanHandlerVIUBufferManager>>& models_uniform_command_bufffers,
    const std::vector<AgModel>& models,
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
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerPresenter - draw_frame] Failed to acquire swap chain image!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerPresenter - draw_frame] Failed to acquire swap chain image!");
    }

    update_uniform_buffer(
        imageIndex,
        logical_device,
        swapchain,
        models_uniform_command_bufffers,
        renderer->m_Camera->m_CameraPosition,
        renderer->m_Camera->m_CameraPosition - renderer->m_Camera->m_CameraOppositeDirection,
        renderer->m_Camera->m_Up,
        models);

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
        // ! ADD TO SEE IT ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

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
    const std::vector<std::shared_ptr<AgVulkanHandlerVIUBufferManager>>& uniforms_buffers,
    const glm::vec3& camera_position,
    const glm::vec3& camera_target,
    const glm::vec3& camera_orientation,
    const std::vector<AgModel>& models
)
{
    Agos::VulkanGraphicsPipeline::UniformBufferObject ubo{};

    // * "I'm an uniform MVP "
    ubo.model = glm::mat4(1.0f);
    ubo.view = glm::lookAt(camera_position, camera_target, camera_orientation);
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchain->get_swapchain_extent().width / (float)swapchain->get_swapchain_extent().height, 0.0625f, 500.0f);
    ubo.proj[1][1] *= -1;


    // * light
    std::vector<size_t> light_models;
    Agos::AgModelExtensionDataLight* extension_data;
    bool no_light_source_among_models = true;

    light_models.reserve(models.size());
    for (size_t i = 0; i < models.size(); i++)
    {
        if (models[i].extension_type == Agos::AgModelExtensionDataType::light_source)
        {
            light_models.push_back(i);
            no_light_source_among_models = false;
        }
    }
    light_models.shrink_to_fit();

    if (no_light_source_among_models)
    {
        extension_data = new(Agos::AgModelExtensionDataLight);
        extension_data->light_position   = glm::vec3(1.0f);
        extension_data->light_color      = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    else
    {
        extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(models[light_models[0]].pExtensionData);
    }    

    for (size_t model = 0; model < models.size(); model++)
    {
        // * "all that glitters is _a light source_ "
        if (models[model].extension_type == Agos::AgModelExtensionDataType::light_source)
        {
            ubo.lightPos    = extension_data->light_position;
            ubo.lightColor  = extension_data->light_color * 1.0f/(models[model].model_data.materials[0].ambient);
        }
        else
        {
            ubo.lightPos    = extension_data->light_position;
            ubo.lightColor  = extension_data->light_color;
        }

        // * model's materials
        ubo.ambient     = models[model].model_data.materials[0].ambient;
        ubo.diffuse     = models[model].model_data.materials[0].diffuse;
        ubo.specular    = models[model].model_data.materials[0].specular;
        ubo.shininess   = models[model].model_data.materials[0].shininess;

        void *data;
        vkMapMemory(logical_device->get_device(), uniforms_buffers[model]->get_uniform_buffers_memory()[current_image], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(logical_device->get_device(), uniforms_buffers[model]->get_uniform_buffers_memory()[current_image]);
    }

    if (no_light_source_among_models)
    {
        delete (extension_data);
    }
}
