#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include AG_VULKAN_INCLUDE
#include AG_GLM_INCLUDE
#include AG_GLM_HASH_TYPES
#include <string>
#include <vector>
#include <array>

namespace Agos
{

namespace VulkanHandler
{

    namespace VulkanModeling
    {

        struct AG_API Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 texCoord;
            glm::vec3 normal;

            Vertex();
            virtual ~Vertex();

            static VkVertexInputBindingDescription                  get_binding_description();
            static std::array<VkVertexInputAttributeDescription, 4> get_attribute_description();

            bool operator==(const Vertex &other) const
            {
                return position == other.position && color == other.color && texCoord == other.texCoord && normal == other.normal;
            }
        }; // struct Vertex

        struct TextureSampler
        {
            VkImage         image;
            VkImageView     imageView;
            VkDeviceMemory  imageMemory;
            VkSampler       imageSampler;

            TextureSampler();
            virtual ~TextureSampler();
        }; // struct TextureSampler


        struct UBO
        {
            struct MVP
            {
                alignas(16) glm::mat4 model;
                alignas(16) glm::mat4 view;
                alignas(16) glm::mat4 proj;
            };

            struct EnvLight
            {
                alignas(16) glm::vec3 lightPos;
                alignas(16) glm::vec3 lightColor;
            };

            struct Materials
            {
                // set = 2
                Agos::VulkanHandler::VulkanModeling::TextureSampler     texture;
                Agos::VulkanHandler::VulkanModeling::TextureSampler     ambtMap;
                Agos::VulkanHandler::VulkanModeling::TextureSampler     diffMap;
                Agos::VulkanHandler::VulkanModeling::TextureSampler     specMap;
                Agos::VulkanHandler::VulkanModeling::TextureSampler     nrmlMap;

                // set = 1
                glm::float32_t  shininess;
                glm::float32_t  ior;        // index of refraction
                glm::float32_t  opacity;    // varies from 0 up to 1
            };
        };

    } // namespace VulkanModeling

} // namespace VulkanHandler

} // namespace Agos


template <>
struct std::hash<Agos::VulkanHandler::VulkanModeling::Vertex>
{
    size_t operator()(Agos::VulkanHandler::VulkanModeling::Vertex const &vertex) const
    {
        size_t h_position = std::hash<glm::vec3>()(vertex.position);
        size_t h_clr = std::hash<glm::vec3>()(vertex.color);
        size_t h_tex = std::hash<glm::vec2>()(vertex.texCoord);
        size_t h_nrm = std::hash<glm::vec3>()(vertex.normal);

        return ((   ( (h_position ^ (h_clr << 1)) >> 1 ) ^ (h_tex << 1)   ) << 1) ^ h_nrm;
    }
};
