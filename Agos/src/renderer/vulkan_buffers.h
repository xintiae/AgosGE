#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"
namespace Agos{
    class AgVulkanHandlerGraphicsPipelineManager;
    namespace VulkanGraphicsPipeline{
        struct Vertex;
    }
}
#include "Agos/src/renderer/vulkan_graphics_pipeline.h"
namespace Agos{
    class AgVulkanHandlerColorDepthRessourcesManager;
}
#include "Agos/src/renderer/vulkan_ressources.h"
namespace Agos{
    class AgVulkanHandlerFramebuffers;
}
#include "Agos/src/renderer/vulkan_framebuffers.h"
#include "Agos/src/renderer/vulkan_command_pool.h"
namespace Agos{
    class AgVulkanHandlerDescriptorManager;
}
#include "Agos/src/renderer/vulkan_descriptor.h"
namespace Agos{
    class AgModel;
}
#include "Agos/src/renderer/model_loader.h"

#include AG_VULKAN_INCLUDE
#include <memory>
#include <vector>
#include <cstdint>


namespace Agos
{
typedef class AG_API AgVulkanHandlerVIUBufferManager
{
private:
    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexBufferMemory;

    VkBuffer m_IndexBuffer;
    VkDeviceMemory m_IndexBufferMemory;

    std::vector<VkBuffer> m_UniformBuffers;
    std::vector<VkDeviceMemory> m_UniformBuffersMemory;

    VkDevice& m_LogicalDeviceReference;
    // buffers simultanés + gestion multiples vertex / index / uniforms buffers
    bool m_VertexBufferTerminated   = false;
    bool m_IndexBufferTerminated    = false;
    bool m_UniformBufferTerminated  = false;
    bool m_Terminated = false;

public:
    AgVulkanHandlerVIUBufferManager();
    AgVulkanHandlerVIUBufferManager(VkDevice& logical_device);
    ~AgVulkanHandlerVIUBufferManager();

    AgResult create_vertex_buffer(
        const std::vector<VulkanGraphicsPipeline::Vertex>& vertices,
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager,
        const std::shared_ptr<AgVulkanHandlerCommandPoolManager>& command_pool_manager);

    AgResult create_index_buffer(
        const std::vector<uint32_t>& indices,
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager,
        const std::shared_ptr<AgVulkanHandlerCommandPoolManager>& command_pool_manager);

    AgResult create_uniform_buffers(
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager);

    AgResult terminate_vertex_buffer();
    AgResult terminate_index_buffer();
    AgResult terminate_uniform_buffers();
    AgResult terminate();

    VkBuffer& get_vertex_buffer();
    VkDeviceMemory& get_vertex_buffer_memory();

    VkBuffer& get_index_buffer();
    VkDeviceMemory& get_index_buffer_memory();

    std::vector<VkBuffer>& get_uniform_buffers();
    std::vector<VkDeviceMemory>& get_uniform_buffers_memory();

public:
    static void create_buffer(
        const VkPhysicalDevice& physical_device,
        const VkDevice& logical_device,
        const std::shared_ptr<AgVulkanHandlerColorDepthRessourcesManager>& color_depth_ressources_manager,
        const VkDeviceSize& size,
        const VkBufferUsageFlags& usage,
        const VkMemoryPropertyFlags& properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    static void copy_buffer(
        const VkDevice& logical_device,
        const VkQueue& queue,
        const VkCommandPool& command_pool,
        const VkBuffer& srcBuffer,
        const VkBuffer& dstBuffer,
        const VkDeviceSize& size);

    static VkCommandBuffer begin_single_time_command(
        const VkDevice& logical_device,
        const VkCommandPool& commandPool);

    static void end_single_time_command(
        const VkDevice& logical_device,
        const VkQueue& graphics_queue,
        const VkCommandPool& command_pool,
        const VkCommandBuffer& commandBuffer);

} AgVulkanHandlerVIUBufferManager;

typedef class AG_API AgVulkanHandlerCommandBufferManager
{
private:
    static std::vector<VkCommandBuffer> m_CommandBuffers;
    static bool m_CommandBuffersTerminated;

    VkDevice& m_LogicalDeviceReference;
    VkCommandPool& m_CommandPoolReference;

public:
    AgVulkanHandlerCommandBufferManager();
    AgVulkanHandlerCommandBufferManager(VkDevice& logical_device);
    AgVulkanHandlerCommandBufferManager(VkCommandPool& command_pool);
    AgVulkanHandlerCommandBufferManager(VkDevice& logical_device, VkCommandPool& command_pool);
    ~AgVulkanHandlerCommandBufferManager();

    AgResult create_command_buffers(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::shared_ptr<AgVulkanHandlerRenderPass>& render_pass,
        const std::shared_ptr<AgVulkanHandlerFramebuffers>& framebuffers_manager,
        const std::shared_ptr<AgVulkanHandlerGraphicsPipelineManager>& graphics_pipeline_manager,
        const std::shared_ptr<AgVulkanHandlerCommandPoolManager>& command_pool_manager,
        const std::shared_ptr<AgVulkanHandlerDescriptorManager>& descriptor_manager,
        const std::vector<std::shared_ptr<AgVulkanHandlerVIUBufferManager>>& models_VIU_buffers,
        const std::vector<AgModel>& models);

    AgResult terminate_command_buffers();
    AgResult terminate();

    std::vector<VkCommandBuffer>& get_command_buffers();

} AgVulkanHandlerCommandBufferManager;

} // namespace Agos
