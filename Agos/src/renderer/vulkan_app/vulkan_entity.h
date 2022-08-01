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

    namespace VulkanEntity
    {

        struct AG_API Vertex
        {
            glm::vec3 position;
            glm::vec3 color;

            glm::vec2 textCoord;
            glm::vec2 ambtCoord;
            glm::vec2 diffCoord;
            glm::vec2 specCoord;
            glm::vec2 nrmlCoord;
            glm::vec2 aplhCoord;

            glm::vec3 normal;
            // ambt, diff, spec and nrml will be computed at render-time

            Vertex()    {}
            ~Vertex()   {}

            static VkVertexInputBindingDescription                  get_binding_description();
            static std::array<VkVertexInputAttributeDescription, 9> get_attribute_description();

            bool operator==(const Vertex &other) const
            {
                return position == other.position && color == other.color && textCoord == other.textCoord && normal == other.normal;
            }
        }; // struct Vertex

        // no need for the physical body of a specified entity to be "beautiful"
        struct AG_API PhysicalVertex
        {
            glm::vec3 position;
            glm::vec3 normal;
        };


        class TextureSampler
        {
        public:
            VkImage         m_Image;
            VkImageView     m_ImageView;
            VkDeviceMemory  m_ImageMemory;
            VkSampler       m_ImageSampler;
            uint16_t        m_MipLevels;

        private:
            bool            m_Terminated                        = false;
            const VkDevice* m_SamplerDeviceReference            = nullptr;
            const VkAllocationCallbacks* m_AllocatorReference   = nullptr;

        public:
            TextureSampler()    {}
            ~TextureSampler()   { critical_assertion(); }

            /**
             * @brief Creates a sampler object. @b NOTE that both specified logical_device and allocator will be referenced to ensure proper destruction of the sampler.
             * 
             * @param image_path path to the texture, to which the sampler will be bounded
             * @param physical_device 
             * @param logical_device @b NOTE that the specified logical_device will be referenced to ensure proper destruction of the sampler.
             * @param graphics_queue 
             * @param allocation_command_pool 
             * @param allocator 
             * @return AgResult AG_SUCCESS if creation successful, AG_FAILED otherwise
             */
            AgResult        create_sampler(     const std::string&              image_path,
                                                const VkPhysicalDevice&         physical_device,
                                                const VkDevice&                 logical_device,
                                                const VkQueue&                  graphics_queue,
                                                const VkCommandPool&            allocation_command_pool,
                                                const VkAllocationCallbacks*    allocator                   );
            /**
             * @brief Destroys a sampler object. @b NOTE that both specified logical_device and allocator in create_sampler is considered as the logical_device reference to ensure proper destruction of the sampler.
             */
            AgResult        terminate_sampler   ();

        private:
            AgResult        create_image        (   const std::string&              image_path,
                                                    const VkPhysicalDevice&         physical_device,
                                                    const VkDevice&                 logical_device,
                                                    const VkQueue&                  graphics_queue,
                                                    const VkCommandPool&            allocation_command_pool,
                                                    const VkAllocationCallbacks*    allocator               );
            AgResult        create_image_view   (   const VkDevice&                 logical_device,
                                                    const VkAllocationCallbacks*    allocator               );
            AgResult        create_image_sampler(   const VkPhysicalDevice&         physical_device,
                                                    const VkDevice&                 logical_device          ,
                                                    const VkAllocationCallbacks*    allocator               );

            void            critical_assertion();
        }; // struct TextureSampler


        namespace UBO
        {
            struct MVP
            {
                // set = 0
                alignas(16) glm::mat4 model;
                alignas(16) glm::mat4 view;
                alignas(16) glm::mat4 proj;
            };

            struct EnvLight
            {
                // set = 0
                alignas(16) glm::vec3 lightPos;
                alignas(16) glm::vec3 lightColor;
            };

            struct Materials
            {
                // set = 1
                glm::float32_t  shininess;
                glm::float32_t  ior;        // index of refraction
                glm::float32_t  opacity;    // varies from 0 up to 1
            };

            struct TexAmbDiffSpecNrmlMaps
            {
                // set = 2
                Agos::VulkanHandler::VulkanEntity::TextureSampler     texture;
                Agos::VulkanHandler::VulkanEntity::TextureSampler     ambtMap;
                Agos::VulkanHandler::VulkanEntity::TextureSampler     diffMap;
                Agos::VulkanHandler::VulkanEntity::TextureSampler     specMap;
                Agos::VulkanHandler::VulkanEntity::TextureSampler     nrmlMap;
            };
        };

    } // namespace VulkanEntity

} // namespace VulkanHandler

} // namespace Agos


template <>
struct std::hash<Agos::VulkanHandler::VulkanEntity::Vertex>
{
    size_t operator()(Agos::VulkanHandler::VulkanEntity::Vertex const &vertex) const
    {
        size_t h_pos = std::hash<glm::vec3>()(vertex.position);
        size_t h_clr = std::hash<glm::vec3>()(vertex.color);
        size_t h_tex = std::hash<glm::vec2>()(vertex.textCoord);
        size_t h_nrm = std::hash<glm::vec3>()(vertex.normal);

        return ((   ( (h_pos ^ (h_clr << 1)) >> 1 ) ^ (h_tex << 1)   ) << 1) ^ h_nrm;
    }
};
