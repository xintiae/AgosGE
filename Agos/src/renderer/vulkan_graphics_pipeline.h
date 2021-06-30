#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_instance.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"
#include "Agos/src/renderer/vulkan_render_pass.h"
#include "Agos/src/renderer/vulkan_descriptor.h"

#include AG_VULKAN_INCLUDE
#include AG_GLM_INCLUDE
#include <memory>
#include <array>
#include <vector>
#include <string>


namespace Agos
{

namespace VulkanGraphicsPipeline
{
typedef enum AG_API ShaderTypes : int8_t
{
    __Error_type    = -1,
    __None          = 0,
    __Vertex        = 1,
    __Fragment      = 2
} ShaderTypes;

typedef struct AG_API Shader
{
    std::string folder_path;
    std::vector<char> shader_contents;

    ShaderTypes type;
    std::string id;
    std::string id_compiled;
    std::string compile;
} Shader;

typedef struct AG_API Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex &other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
} Vertex;

typedef struct AG_API UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
} UniformBufferObject;
}   // namespace VulkanGraphicsPipeline (within namespace Agos)

typedef class AG_API AgVulkanHandlerGraphicsPipelineManager
{
private:
    VkPipeline m_GraphicsPipeline;
    VkPipelineLayout m_GraphicsPipelineLayout;

    // do I need to repeat myself?... (see Agos/src/renderer/vulkan_swapchain.h)
    VkDevice m_LogicalDeviceReference;
    bool m_Terminated = false;

public:
    AgVulkanHandlerGraphicsPipelineManager();
    AgVulkanHandlerGraphicsPipelineManager(const VkDevice& logical_device);
    ~AgVulkanHandlerGraphicsPipelineManager();

    AgResult create_graphics_pipeline(
        const std::string& shaders_path,
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::shared_ptr<AgVulkanHandlerRenderPass>& render_pass,
        const std::shared_ptr<AgVulkanHandlerDescriptorManager>& descriptor);
    AgResult terminate();

private:
    VkShaderModule create_shader_module(
        const std::string& shader_folder,
        const VkDevice& logical_device);
    VulkanGraphicsPipeline::Shader compile_shader(const std::string& shader_folder_path);
    const std::vector<char> read_file(const std::string& file_path);
};

}   // namespace Agos