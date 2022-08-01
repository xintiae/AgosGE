#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#ifdef AG_GRAPHICS_API_VULKAN
    #include "Agos/src/renderer/vulkan_app/vulkan_entity.h"
    using namespace Agos::VulkanHandler::VulkanEntity;
#endif
#ifdef AG_GRAPHICS_API_OPENGL
    #include "Agos/src/renderer/opengl_app/opengl_entity.h"
    // using namespace Agos::OpenGLHandler::OpenGLEntity;
#endif


#include <string>
#include <cstdint>
#include <atomic>

#include AG_TINY_OBJ_LOADER_INCLUDE
#include AG_GLM_INCLUDE

namespace Agos
{
namespace Entities
{

    namespace EntityData
    {
        struct AG_API Materials
        {
            bool            has_mtl;
            bool            has_texture;
            bool            has_ambiant;
            bool            has_diffuse;
            bool            has_specular;
            bool            has_normal;
            bool            has_alpha;

            std::string     texture_path;
            std::string     ambiant_path;
            std::string     diffuse_path;
            std::string     specular_path;
            std::string     normal_path;
            std::string     alpha_path;

            TextureSampler  texture;
            TextureSampler  ambtMap;
            TextureSampler  diffMap;
            TextureSampler  specMap;
            TextureSampler  nrmlMap;
            TextureSampler  alphMap;

            glm::float32_t  shininess;
            glm::float32_t  ior;        // index of refraction
            glm::float32_t  opacity;    // varies from 0 up to 1 
        };

        struct AG_API Properties
        {
            enum EntityType : uint8_t
            {
                __Generic       = 0,
                __Light_Src     = 1,
                __Max_Enum      = 255
            } entity_type;

            struct Light
            {
                glm::vec3   position;
                glm::vec3   color;
            } light;

            struct PhysicalBody
            {
                float       mass;
                float       charge;

                // glm::vec3   position; // = verticices[0].position;
                glm::vec3   momentum;   // m*v
                glm::vec3   speed;
                glm::vec3   acceleration;

                // no need for "pretty" physical boddies
                std::vector<PhysicalVertex> vertices;
                std::vector<uint32_t>       indices;
                // elasticity, literally anything else...
            } physical_body;
        };

        // ** Entity Data =======================================
        struct AG_API Data
        {
            std::string     entity_name;
            std::string     obj_file_path;

            glm::vec3       translation;
            glm::vec3       rotation;
            glm::vec4       overall_color;

            std::vector<Vertex>                     vertices;
            std::vector<uint32_t>                   indices;
            Agos::Entities::EntityData::Materials   materials;
            Agos::Entities::EntityData::Properties  properties;
        };
        // ** Entity Data =======================================
    }   // namespace EntityData (within namespace Agos::Entities)

    // ** AgosGE Entity =========================================
    class AG_API Entity
    {
    private:
        size_t                              m_Id;               // = 0
        Agos::Entities::EntityData::Data    m_Data;

        bool                                m_ShouldBeShown;    // = true
        bool                                m_GPULoaded;        // = false
        bool                                m_EntityDestroyed;  // = false

    public:
        // * = * = * = * Constructors, destructors * = * = * = *
        Entity  ();
        Entity  (const size_t& id);
        Entity  (const size_t& id, const Entities::EntityData::Data& data);
        Entity  (const Entity& other);
        Entity  (Entity&& other);
        virtual ~Entity();

        Entity&     operator=(const Entity& other);
        Entity&     operator=(Entity&& other);
        // * = * = * = * Constructors, destructors * = * = * = *


        // * = * = * = * Misc * = * = * = *
        size_t&                             get_entity_id           ()          { return m_Id;              }
        Agos::Entities::EntityData::Data&   get_entity_data         ()          { return m_Data;            }
        bool&                               entity_destroyed        ()          { return m_EntityDestroyed; }
        // true : entity loaded onto the GPU | false : entity isn't loaded onto the GPU
        bool&                               get_entity_gpu_status   ()          { return m_GPULoaded;       }
        bool&                               should_be_shown         ()          { return m_ShouldBeShown;   }

        const size_t&                       get_entity_id           ()  const   { return m_Id;              }
        const bool&                         entity_destroyed        ()  const   { return m_EntityDestroyed; }
        const bool&                         get_entity_gpu_status   ()  const   { return m_GPULoaded;       }
        const bool&                         should_be_shown         ()  const   { return m_ShouldBeShown;   }
        // * = * = * = * Misc * = * = * = *

        // * = * = * = * Entity manipulation * = * = * = *
        AgResult    show();
        AgResult    hide();
        AgResult    destroy_entity();
        // * = * = * = * Entity manipulation * = * = * = *


    private:
        // creates : 
        // (Vulkan-based renderer) VkImage, VkImageView, VkDeviceMemory, VkSampler
        // (OpenGL-based renderer) <?> - still WIP, remember?
        // AgResult    load_texture        (const std::string& texture_path);
        // AgResult    load_lighting_map   (const std::string& ambiant_map, const std::string& diffuse_path, const std::string& specular_map, const std::string& normal_map);

        // TO IMPLEMENT
        // AgResult    destroy_texture();
        // AgResult    destroy_lighting_map();
    };
    // ** AgosGE Entity =========================================


/*
    struct AG_API EntityManager
    {
        //  * @brief main Agos function to parse an .obj file with its corresponding .mtl file
        //  * @param Entity Agos::Entities::Entity containing @b both informations to Entity.path_to_obj_file @a and Entity.path_to_texture_file
        //  * @param color Default color to apply to the entire Entity
        //  * @return A filled in with Agos::Entities::EntityManager struct with informations from the .obj
        static AgResult     load_Entity         (Agos::Entities::Entity& Entity, const glm::vec3& default_polygons_color = glm::vec3(1.0f)      );
        static AgResult     translate           (Agos::Entities::Entity& Entity, const glm::vec3& translation                                   );
        static AgResult     scale               (Agos::Entities::Entity& Entity, const glm::vec3& translation                                   );
        static AgResult     rotate              (Agos::Entities::Entity& Entity, const glm::vec3& rotation_axis, const float& angle_radians     );
        static AgResult     set_polygons_color  (Agos::Entities::Entity& Entity, const glm::vec3& polygons_color                                );
 
        static AgResult     set_obj_path        (Agos::Entities::Entity& Entity, const std::string& obj_file_path       );
        static AgResult     set_texture_path    (Agos::Entities::Entity& Entity, const std::string& texture_path        );
        static AgResult     set_ambiant_path    (Agos::Entities::Entity& Entity, const std::string& ambiant_map_path    );
        static AgResult     set_diffuse_path    (Agos::Entities::Entity& Entity, const std::string& diffuse_map_path    );
        static AgResult     set_specular_path   (Agos::Entities::Entity& Entity, const std::string& specular_map_path   );
        static AgResult     set_normal_map      (Agos::Entities::Entity& Entity, const std::string& normal_map_path     );
        static AgResult     set_lighting_map    (Agos::Entities::Entity& Entity, const std::string& ambiant_map_path,
                                                                               const std::string& diffuse_map_path,
                                                                               const std::string& specular_map_path     );

        static AgResult     set_light_source    (Agos::Entities::Entity& Entity, const glm::vec3& light_color           );
        static AgResult     set_physical_body   (Agos::Entities::Entity& Entity                                         );

        static AgResult     display_position_vector         (Agos::Entities::Entity& Entity     );
        static AgResult     display_speed_vector            (Agos::Entities::Entity& Entity     );
        static AgResult     display_acceleration_vector     (Agos::Entities::Entity& Entity     );
        static AgResult     hide_position_vector            (Agos::Entities::Entity& Entity     );
        static AgResult     hide_speed_vector               (Agos::Entities::Entity& Entity     );
        static AgResult     hide_acceleration_vector        (Agos::Entities::Entity& Entity     );
    };
*/

}   // namespace Entity (within namespace Agos::Entities)
}   // namespace Agos
