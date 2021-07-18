#include "Agos/src/renderer/vulkan_buffers.h"

#include "Agos/src/logger/logger.h"


Agos::AgVulkanHandlerVIUBufferManager::AgVulkanHandlerVIUBufferManager()
    : m_LogicalDeviceReference(AG_DEFAULT_LOGICAL_DEVICE_REFERENCE)
{
}

Agos::AgVulkanHandlerVIUBufferManager::AgVulkanHandlerVIUBufferManager(VkDevice& logical_device)
    : m_LogicalDeviceReference(logical_device)
{
}

Agos::AgVulkanHandlerVIUBufferManager::~AgVulkanHandlerVIUBufferManager()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerVIUBufferManager::create_vertex_buffer(
    const std::vector<Agos::VulkanGraphicsPipeline::Vertex>& vertices,
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager,
    const std::shared_ptr<AgVulkanHandlerCommandPoolManager>& command_pool_manager
)
{
    m_LogicalDeviceReference = logical_device->get_device();

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(
        physical_device->get_device(),
        logical_device->get_device(),
        color_depth_ressources_manager,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *data;
    vkMapMemory(logical_device->get_device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(logical_device->get_device(), stagingBufferMemory);

    create_buffer(
        physical_device->get_device(),
        logical_device->get_device(),
        color_depth_ressources_manager,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_VertexBuffer,
        m_VertexBufferMemory);

    copy_buffer(
        logical_device->get_device(),
        logical_device->get_graphics_queue(),
        command_pool_manager->get_command_pool(),
        stagingBuffer,
        m_VertexBuffer,
        bufferSize);

    vkDestroyBuffer(logical_device->get_device(), stagingBuffer, nullptr);
    vkFreeMemory(logical_device->get_device(), stagingBufferMemory, nullptr);

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerVIUBufferManager - create_vertex_buffer] Created vertex buffer!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerVIUBufferManager::create_index_buffer(
    const std::vector<uint32_t>& indices,
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager,
    const std::shared_ptr<AgVulkanHandlerCommandPoolManager>& command_pool_manager
)
{
    m_LogicalDeviceReference = logical_device->get_device();

    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(
        physical_device->get_device(),
        logical_device->get_device(),
        color_depth_ressources_manager,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *data;
    vkMapMemory(logical_device->get_device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(logical_device->get_device(), stagingBufferMemory);

    create_buffer(
        physical_device->get_device(),
        logical_device->get_device(),
        color_depth_ressources_manager,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_IndexBuffer,
        m_IndexBufferMemory);

    copy_buffer(
        logical_device->get_device(),
        logical_device->get_graphics_queue(),
        command_pool_manager->get_command_pool(),
        stagingBuffer,
        m_IndexBuffer,
        bufferSize);

    vkDestroyBuffer(logical_device->get_device(), stagingBuffer, nullptr);
    vkFreeMemory(logical_device->get_device(), stagingBufferMemory, nullptr);

    AG_CORE_INFO("[Vulkan/AgVulkanHandlerVIUBufferManager - create_index_buffer] Created index buffer!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerVIUBufferManager::create_uniform_buffers(
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
    const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager
)
{
    m_LogicalDeviceReference = logical_device->get_device();

    VkDeviceSize bufferSize = sizeof(Agos::VulkanGraphicsPipeline::UniformBufferObject);

    m_UniformBuffers.resize(swapchain->get_swapchain_images().size());
    m_UniformBuffersMemory.resize(swapchain->get_swapchain_images().size());

    for (size_t i = 0; i < swapchain->get_swapchain_images().size(); i++)
    {
        create_buffer(
            physical_device->get_device(),
            logical_device->get_device(),
            color_depth_ressources_manager,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_UniformBuffers[i],
            m_UniformBuffersMemory[i]);
    }
    AG_CORE_INFO("[Vulkan/AgVulkanHandlerVIUBufferManager - create_uniform_buffer] Created uniform buffer!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerVIUBufferManager::terminate_vertex_buffer()
{
    if (!m_VertexBufferTerminated)
    {
        vkDestroyBuffer(m_LogicalDeviceReference, m_VertexBuffer, nullptr);
        vkFreeMemory(m_LogicalDeviceReference, m_VertexBufferMemory, nullptr);
        AG_CORE_INFO("[Vulkan/AgVulkanHanlderBufferManager - terminate_vertex_buffer] Destroyed vertex buffer; freed vertex buffer memory!");
        m_VertexBufferTerminated = true;
        return AG_SUCCESS;
    }
    return AG_VERTEX_BUFFER_ALREADY_FREED;
}

Agos::AgResult Agos::AgVulkanHandlerVIUBufferManager::terminate_index_buffer()
{
    if (!m_IndexBufferTerminated)
    {
        vkDestroyBuffer(m_LogicalDeviceReference, m_IndexBuffer, nullptr);
        vkFreeMemory(m_LogicalDeviceReference, m_IndexBufferMemory, nullptr);
        AG_CORE_INFO("[Vulkan/AgVulkanHanlderBufferManager - terminate_index_buffer] Destroyed index buffer; freed index buffer memory!");
        m_IndexBufferTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INDEX_BUFFER_ALREADY_FREED;
}

Agos::AgResult Agos::AgVulkanHandlerVIUBufferManager::terminate_uniform_buffers()
{
    if (!m_UniformBufferTerminated)
    {
        for (size_t i = 0; i < m_UniformBuffers.size(); i++)    // /!\t
        {
            vkDestroyBuffer(m_LogicalDeviceReference, m_UniformBuffers[i], nullptr);
            vkFreeMemory(m_LogicalDeviceReference, m_UniformBuffersMemory[i], nullptr);
        }
        AG_CORE_INFO("[Vulkan/AgVulkanHanlderBufferManager - terminate_uniform_buffers] Destroyed uniform buffers; freed uniform buffers memory!");
        m_UniformBufferTerminated = true;
        return AG_SUCCESS;
    }
    return AG_UNIFORM_BUFFERS_ALREADY_FREED;
}

Agos::AgResult Agos::AgVulkanHandlerVIUBufferManager::terminate()
{
    if (!m_Terminated)
    {
        terminate_uniform_buffers();
        terminate_index_buffer();
        terminate_vertex_buffer();
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerVIUBufferManager - terminate] Terminated instance!");
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

VkBuffer& Agos::AgVulkanHandlerVIUBufferManager::get_vertex_buffer()
{
    return m_VertexBuffer;
}

VkDeviceMemory& Agos::AgVulkanHandlerVIUBufferManager::get_vertex_buffer_memory()
{
    return m_VertexBufferMemory;
}

VkBuffer& Agos::AgVulkanHandlerVIUBufferManager::get_index_buffer()
{
    return m_IndexBuffer;
}

VkDeviceMemory& Agos::AgVulkanHandlerVIUBufferManager::get_index_buffer_memory()
{
    return m_IndexBufferMemory;
}

std::vector<VkBuffer>& Agos::AgVulkanHandlerVIUBufferManager::get_uniform_buffers()
{
    return m_UniformBuffers;
}

std::vector<VkDeviceMemory>& Agos::AgVulkanHandlerVIUBufferManager::get_uniform_buffers_memory()
{
    return m_UniformBuffersMemory;
}

std::vector<VkCommandBuffer>& Agos::AgVulkanHandlerCommandBufferManager::get_command_buffers()
{
    return m_CommandBuffers;
}

void Agos::AgVulkanHandlerVIUBufferManager::create_buffer(
    const VkPhysicalDevice& physical_device,
    const VkDevice& logical_device,
    const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager,
    const VkDeviceSize& size,
    const VkBufferUsageFlags& usage,
    const VkMemoryPropertyFlags& properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logical_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        AG_CORE_ERROR("[Vulkan/AgVulkanHandlerVIUBufferManager - create_buffer] Failed to create buffer!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerVIUBufferManager - create_buffer] Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logical_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = color_depth_ressources_manager->find_memory_type(physical_device, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logical_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        AG_CORE_ERROR("[Vulkan/AgVulkanHandlerVIUBufferManager - create_buffer] Failed to allocate buffer memory!");
        throw std::runtime_error("[Vulkan/AgVulkanHandlerVIUBufferManager - create_buffer] Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(logical_device, buffer, bufferMemory, 0);
}

void Agos::AgVulkanHandlerVIUBufferManager::copy_buffer(
    const VkDevice& logical_device,
    const VkQueue& queue,
    const VkCommandPool& command_pool,
    const VkBuffer& srcBuffer,
    const VkBuffer& dstBuffer,
    const VkDeviceSize& size)
{
    VkCommandBuffer commandBuffer = begin_single_time_command(logical_device, command_pool);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    end_single_time_command(
        logical_device,
        queue,
        command_pool,
        commandBuffer);
}

VkCommandBuffer Agos::AgVulkanHandlerVIUBufferManager::begin_single_time_command(
    const VkDevice& logical_device,
    const VkCommandPool& commandPool
)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logical_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Agos::AgVulkanHandlerVIUBufferManager::end_single_time_command(
    const VkDevice& logical_device,
    const VkQueue& graphics_queue,
    const VkCommandPool& command_pool,
    const VkCommandBuffer& commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(logical_device, command_pool, 1, &commandBuffer);
}


std::vector<VkCommandBuffer> Agos::AgVulkanHandlerCommandBufferManager::m_CommandBuffers;
bool Agos::AgVulkanHandlerCommandBufferManager::m_CommandBuffersTerminated = false;

Agos::AgVulkanHandlerCommandBufferManager::AgVulkanHandlerCommandBufferManager()
    : m_LogicalDeviceReference(AG_DEFAULT_LOGICAL_DEVICE_REFERENCE), m_CommandPoolReference(AG_DEFAULT_COMMAND_POOL_REFERENCE)
{
}

Agos::AgVulkanHandlerCommandBufferManager::AgVulkanHandlerCommandBufferManager(VkDevice& logical_device)
    : m_LogicalDeviceReference(logical_device), m_CommandPoolReference(AG_DEFAULT_COMMAND_POOL_REFERENCE)
{
}

Agos::AgVulkanHandlerCommandBufferManager::AgVulkanHandlerCommandBufferManager(VkCommandPool& command_pool)
    : m_LogicalDeviceReference(AG_DEFAULT_LOGICAL_DEVICE_REFERENCE), m_CommandPoolReference(command_pool)
{
}

Agos::AgVulkanHandlerCommandBufferManager::AgVulkanHandlerCommandBufferManager(VkDevice& logical_device, VkCommandPool& command_pool)
    : m_LogicalDeviceReference(logical_device), m_CommandPoolReference(command_pool)
{
}

Agos::AgVulkanHandlerCommandBufferManager::~AgVulkanHandlerCommandBufferManager()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerCommandBufferManager::create_command_buffers(
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
    const std::shared_ptr<AgVulkanHandlerRenderPass>& render_pass,
    const std::shared_ptr<AgVulkanHandlerFramebuffers>& framebuffers_manager,
    const std::shared_ptr<AgVulkanHandlerGraphicsPipelineManager>& graphics_pipeline_manager,
    const std::shared_ptr<AgVulkanHandlerCommandPoolManager>& command_pool_manager,
    const std::shared_ptr<AgVulkanHandlerDescriptorManager>& descriptor_manager,
    const std::vector<std::shared_ptr<AgVulkanHandlerVIUBufferManager>>& models_VIU_buffers,
    const std::vector<AgModel>& models
)
{
    m_LogicalDeviceReference = logical_device->get_device();
    m_CommandPoolReference = command_pool_manager->get_command_pool();

    m_CommandBuffers.resize(framebuffers_manager->get_swapchain_framebuffers().size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool_manager->get_command_pool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

    if (vkAllocateCommandBuffers(logical_device->get_device(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("[Vulkan/AgVulkanHandlerVIUBufferManager - create_command_buffers] Failed to allocate command buffers!");
    }

    for (size_t current_command_buffer = 0; current_command_buffer < m_CommandBuffers.size(); current_command_buffer++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(m_CommandBuffers[current_command_buffer], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("[Vulkan/AgVulkanHandlerVIUBufferManager - create_command_buffers] Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = render_pass->get_render_pass();
        renderPassInfo.framebuffer = framebuffers_manager->get_swapchain_framebuffers()[current_command_buffer];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchain->get_swapchain_extent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_CommandBuffers[current_command_buffer], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_CommandBuffers[current_command_buffer], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_manager->get_graphics_pipeline());

        for (size_t current_VIU_buffer = 0; current_VIU_buffer < models_VIU_buffers.size(); current_VIU_buffer++)
        {
            VkBuffer vertexBuffers[] = { models_VIU_buffers[current_VIU_buffer]->get_vertex_buffer() };
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(m_CommandBuffers[current_command_buffer], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(m_CommandBuffers[current_command_buffer], models_VIU_buffers[current_VIU_buffer]->get_index_buffer(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(
                m_CommandBuffers[current_command_buffer],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphics_pipeline_manager->get_graphics_pipeline_layout(),
                0, 1,
                &descriptor_manager->get_descriptor_sets(current_VIU_buffer)[current_command_buffer],
                0, nullptr);

            vkCmdDrawIndexed(m_CommandBuffers[current_command_buffer], static_cast<uint32_t>(models[current_VIU_buffer].model_data.indices.size()), 1, 0, 0, 0);

        }

        vkCmdEndRenderPass(m_CommandBuffers[current_command_buffer]);

        if (vkEndCommandBuffer(m_CommandBuffers[current_command_buffer]) != VK_SUCCESS)
        {
            AG_CORE_CRITICAL("[Vulkan/AgVulkanHanderBufferManager - create_command_buffers] Failed to record command buffer!");
            throw std::runtime_error("[Vulkan/AgVulkanHanderBufferManager - create_command_buffers] Failed to record command buffer!");
        }
    }

    AG_CORE_INFO("[Vulkan/AgVulkanHanderBufferManager - create_command_buffers] Recored command buffers");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerCommandBufferManager::terminate()
{
    return terminate_command_buffers();
}

Agos::AgResult Agos::AgVulkanHandlerCommandBufferManager::terminate_command_buffers()
{
    if (!m_CommandBuffersTerminated)
    {
        vkFreeCommandBuffers(
            m_LogicalDeviceReference,
            m_CommandPoolReference,
            static_cast<uint32_t>(m_CommandBuffers.size()),
            m_CommandBuffers.data());
        AG_CORE_INFO("[Vulkan/AgVulkanHanlderBufferManager - terminate_command_buffers] Freed command buffers!");
        m_CommandBuffersTerminated = true;
        return AG_SUCCESS;
    }
    return AG_COMMAND_BUFFERS_ALREADY_FREED;
}
