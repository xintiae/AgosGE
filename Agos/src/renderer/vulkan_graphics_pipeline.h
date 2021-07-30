#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_instance.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"
namespace Agos{
    class AgVulkanHandlerRenderPass;
}
#include "Agos/src/renderer/vulkan_render_pass.h"
namespace Agos{
    class AgVulkanHandlerDescriptorManager;
}
#include "Agos/src/renderer/vulkan_descriptor.h"

#include AG_VULKAN_INCLUDE
#include AG_GLM_HASH_TYPES
#include AG_GLM_INCLUDE
#include <memory>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>


namespace Agos
{

namespace VulkanGraphicsPipeline
{
enum AG_API ShaderTypes : int8_t
{
    __Error_type    = -1,
    __None          = 0,
    __Vertex        = 1,
    __Fragment      = 2
};  // enum ShaderTypes

struct AG_API Shader
{
    std::string folder_path;
    std::vector<char> shader_contents;

    ShaderTypes type;
    std::string id;
    std::string id_compiled;
    std::string compile;
};  // struct Shader

struct AG_API Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

    static VkVertexInputBindingDescription get_binding_description()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> get_attribute_description()
    {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

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

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, normal);

        return attributeDescriptions;
    }

    bool operator==(const Vertex &other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
    }
};  // struct Vertex
}   // namespace VulkanGraphicsPipeline (within namespace Agos)
}   // namespace Agos

namespace std
{
    template <>
    struct hash<Agos::VulkanGraphicsPipeline::Vertex>
    {
        size_t operator()(Agos::VulkanGraphicsPipeline::Vertex const &vertex) const
        {
            size_t h_pos = hash<glm::vec3>()(vertex.pos);
            size_t h_clr = hash<glm::vec3>()(vertex.color);
            size_t h_tex = hash<glm::vec2>()(vertex.texCoord);
            size_t h_nrm = hash<glm::vec3>()(vertex.normal);


            return ((   ( (h_pos ^ (h_clr << 1)) >> 1 ) ^ (h_tex << 1)   ) << 1) ^ h_nrm;
        }
    };
}


namespace Agos
{
class AG_API AgVulkanHandlerGraphicsPipelineManager
{
private:
    VkPipeline m_GraphicsPipeline;
    VkPipelineLayout m_GraphicsPipelineLayout;

    VkDevice& m_LogicalDeviceReference;
    bool m_Terminated = false;

public:
    AgVulkanHandlerGraphicsPipelineManager();
    AgVulkanHandlerGraphicsPipelineManager(VkDevice& logical_device);
    ~AgVulkanHandlerGraphicsPipelineManager();

    AgResult create_graphics_pipeline(
        const std::string& shaders_path,
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
        const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
        const std::shared_ptr<AgVulkanHandlerRenderPass>& render_pass,
        const std::shared_ptr<AgVulkanHandlerDescriptorManager>& descriptor);
    AgResult terminate(const bool& mark_as_terminated = true);

    VkPipeline& get_graphics_pipeline();
    VkPipelineLayout& get_graphics_pipeline_layout();

private:
    VkShaderModule create_shader_module(
        const std::string& shader_folder,
        const VkDevice& logical_device);
    VulkanGraphicsPipeline::Shader compile_shader(const std::string& shader_folder_path);
    AgResult clear_compiled_shader(const VulkanGraphicsPipeline::Shader& shader);
};  // class AgVulkanHandlerGraphicsPipelineManager

}   // namespace Agos