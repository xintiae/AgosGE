#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_instance.h"
#include "Agos/src/renderer/vulkan_physical_device.h"
#include "Agos/src/renderer/vulkan_logical_device.h"
#include "Agos/src/renderer/vulkan_swapchain.h"

#include AG_VULKAN_INCLUDE
#include AG_GLM_INCLUDE
#include <memory>
#include <array>
#include <vector>


namespace Agos
{

namespace VulkanDescriptors
{
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
}   // namespace VulkanDescriptors (within namespace Agos)

typedef class AG_API AgVulkanHandlerDescriptorManager
{
private:
    VkDescriptorSetLayout m_DescriptorSetLayout;

    // do I need to repeat myself?... (see Agos/src/renderer/vulkan_swapchain.h)
    VkDevice m_LogicalDeviceReference;
    bool m_Terminated = false;

public:
    AgVulkanHandlerDescriptorManager();
    AgVulkanHandlerDescriptorManager(const VkDevice& logical_device);
    ~AgVulkanHandlerDescriptorManager();

    AgResult create_descriptor_set_layout(
        const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device
    );
    AgResult terminate();

} AgVulkanHandlerDescriptorManager;
};
