#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#ifdef AG_GRAPHICS_API_VULKAN
    #include "Agos/src/renderer/vulkan_app/vulkan_modeling.h"
    using namespace Agos::VulkanHandler::VulkanModeling;
#endif
#ifdef AG_GRAPHICS_API_OPENGL
    #include "Agos/src/renderer/opengl_app/opengl_modeling.h"

#endif


#include <string>
#include <cstdint>

#include AG_TINY_OBJ_LOADER_INCLUDE
#include AG_GLM_INCLUDE

namespace Agos
{

namespace Modeling
{

    namespace ModelData
    {
        struct AG_API Materials
        {
            // std::string     texture_path;
            // std::string     ambiant_path;
            // std::string     diffuse_path;
            // std::string     specular_path;

            TextureSampler  texture;
            TextureSampler  ambtMap;
            TextureSampler  diffMap;
            TextureSampler  specMap;
            TextureSampler  nrmlMap;

            glm::float32_t  shininess;
            glm::float32_t  ior;        // index of refraction
            glm::float32_t  opacity;    // varies from 0 up to 1 
        };

        struct AG_API Properties
        {
            struct Light
            {
                glm::vec3   position;
                glm::vec3   color;
            };

            struct PhysicalBody
            {
                float   mass;
                float   charge;
                std::vector<Vertex>     vertices;
                std::vector<uint32_t>   indices;
                // elasticity, literally anything else...
            };
        };

        // ** Model Data =======================================
        struct AG_API Data
        {
            std::string     obj_file_path;
            std::string     texture_path;
            std::string     ambiant_path;
            std::string     diffuse_path;
            std::string     specular_path;
            std::string     normal_path;

            std::vector<Vertex>                     vertices;
            std::vector<uint32_t>                   indices;
            Agos::Modeling::ModelData::Materials    materials;
            Agos::Modeling::ModelData::Properties   properties;
        };
    }   // namespace ModelData (within namespace Agos::Modeling)

    // ** AgosGE Model =========================================
    class AG_API Model
    {
    private:
        std::string                         m_Id;
        Agos::Modeling::ModelData::Data     m_ModelData;

    public:

        explicit    Model();
        explicit    Model(const Model& other)      = delete;
        explicit    Model(Model&& other)           = delete;
        virtual    ~Model();
        Model&      operator=(const Model& other)    = delete;
        Model&      operator=(Model&& other)         = delete;

        // creates : 
        // (Vulkan-based renderer) VkImage, VkImageView, VkDeviceMemory, VkSampler
        // (OpenGL-based renderer) <?>
        AgResult    load_texture        (const std::string& texture_path);
        AgResult    load_lighting_map   (const std::string& ambiant_map, const std::string& diffuse_path, const std::string& specular_map);

        AgResult    destroy_texture();
        AgResult    destroy_lighting_map();
    };

    struct AG_API ModelManager
    {
        //  * @brief main Agos function to parse an .obj file with its corresponding .mtl file
        //  * @param model Agos::Modeling::Model containing @b both informations to model.path_to_obj_file @a and model.path_to_texture_file
        //  * @param color Default color to apply to the entire model
        //  * @return A filled in with Agos::Modeling::ModelData struct with informations from the .obj
        static AgResult     load_model          (Agos::Modeling::Model& model, const glm::vec3& polygons_color = glm::vec3(1.0f)            );
        static AgResult     translate           (Agos::Modeling::Model& model, const glm::vec3& translation                                 );
        static AgResult     scale               (Agos::Modeling::Model& model, const glm::vec3& translation                                 );
        static AgResult     rotate              (Agos::Modeling::Model& model, const glm::vec3& rotation_axis, const float& angle_radians   );
        static AgResult     set_polygons_color  (Agos::Modeling::Model& model, const glm::vec3& polygons_color                              );
 
        static AgResult     set_obj_path        (Agos::Modeling::Model& model, const std::string& obj_file_path         );
        static AgResult     set_texture_path    (Agos::Modeling::Model& model, const std::string& texture_path          );
        static AgResult     set_ambiant_path    (Agos::Modeling::Model& model, const std::string& ambiant_map_path      );
        static AgResult     set_diffuse_path    (Agos::Modeling::Model& model, const std::string& diffuse_map_path      );
        static AgResult     set_specular_path   (Agos::Modeling::Model& model, const std::string& specular_map_path     );
        static AgResult     set_normal_map      (Agos::Modeling::Model& model, const std::string& normal_map_path       );
        static AgResult     set_lighting_map    (Agos::Modeling::Model& model, const std::string& ambiant_map_path  ,
                                                                               const std::string& diffuse_map_path  ,
                                                                               const std::string& specular_map_path     );

        static AgResult     set_light_source    (Agos::Modeling::Model& model, const glm::vec3& light_color             );
        static AgResult     set_physical_body   (Agos::Modeling::Model& model                                           );

        static AgResult     display_position_vector         (Agos::Modeling::Model& model   );
        static AgResult     display_speed_vector            (Agos::Modeling::Model& model   );
        static AgResult     display_acceleration_vector     (Agos::Modeling::Model& model   );
        static AgResult     hide_position_vector            (Agos::Modeling::Model& model   );
        static AgResult     hide_speed_vector               (Agos::Modeling::Model& model   );
        static AgResult     hide_acceleration_vector        (Agos::Modeling::Model& model   );
    };

}   // namespace Modeling (within namespace Agos::Modeling)

}   // namespace Agos
