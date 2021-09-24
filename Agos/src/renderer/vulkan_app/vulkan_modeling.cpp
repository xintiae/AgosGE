#include "Agos/src/renderer/vulkan_app/vulkan_modeling.h"


VkVertexInputBindingDescription Agos::VulkanHandler::VulkanModeling::Vertex::get_binding_description()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding      = 0;
    bindingDescription.stride       = sizeof(Vertex);
    bindingDescription.inputRate    = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> Agos::VulkanHandler::VulkanModeling::Vertex::get_attribute_description()
{
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

    attributeDescriptions[0].binding    = 0;
    attributeDescriptions[0].location   = 0;
    attributeDescriptions[0].format     = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset     = offsetof(Vertex, position);

    attributeDescriptions[1].binding    = 0;
    attributeDescriptions[1].location   = 1;
    attributeDescriptions[1].format     = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset     = offsetof(Vertex, color);

    attributeDescriptions[2].binding    = 0;
    attributeDescriptions[2].location   = 2;
    attributeDescriptions[2].format     = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset     = offsetof(Vertex, texCoord);

    attributeDescriptions[3].binding    = 0;
    attributeDescriptions[3].location   = 3;
    attributeDescriptions[3].format     = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset     = offsetof(Vertex, normal);

    return attributeDescriptions;
}

namespace std
{
    template <>
    struct hash<Agos::VulkanHandler::VulkanModeling::Vertex>
    {
        size_t operator()(Agos::VulkanHandler::VulkanModeling::Vertex const &vertex) const
        {
            size_t h_pos = hash<glm::vec3>()(vertex.position);
            size_t h_clr = hash<glm::vec3>()(vertex.color);
            size_t h_tex = hash<glm::vec2>()(vertex.texCoord);
            size_t h_nrm = hash<glm::vec3>()(vertex.normal);


            return ((   ( (h_pos ^ (h_clr << 1)) >> 1 ) ^ (h_tex << 1)   ) << 1) ^ h_nrm;
        }
    };
}
