#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_graphics_pipeline.h"

#include "Agos/src/renderer/modeling/lighting_map.h"

#include <string>
#include <cstdint>

#include AG_TINY_OBJ_LOADER_INCLUDE
#include AG_GLM_INCLUDE

namespace Agos
{

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// consider those to be FLAGS
enum AG_API AgModelExtensionDataType : uint16_t
{
    none            = 0,
    light_source    = 1,
    lighting_map    = 2,
    max_enum        = 65535
};

struct AG_API AgModelExtensionDataLight
{
    glm::vec3 light_position;
    glm::vec3 light_color;
};

struct AG_API AgModelExtensionDataLightingMap
{
    AgModelDataDiffuseMap   diffuse_map;
    AgModelDataSpecularMap  specular_map;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct AG_API AgModelDataMaterial
{
    glm::vec3   ambient;
    glm::vec3   diffuse;
    glm::vec3   specular;
    float       shininess;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct AG_API AgModelData
{
    std::vector<VulkanGraphicsPipeline::Vertex>     vertices;
    std::vector<uint32_t>                           indices;
    glm::vec3                                       translation = glm::vec3(0.0f);
    std::vector<AgModelDataMaterial>                materials;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct AG_API AgModel
{
public:
    std::string id;
    std::string path_to_obj_file;
    std::string path_to_texture_file;

    AgModelData model_data;
    AgModelExtensionDataType extension_type = AgModelExtensionDataType::none;
    void* pExtensionData                    = NULL;

    AgModel();
    AgModel(const AgModel& other);
    AgModel(AgModel&& other);
    ~AgModel();

    AgModel& operator=(const AgModel& other);
    AgModel& operator=(AgModel&&);

private:
    template <uint16_t extension_type>
    static void create_model_extension(AgModel& model);
    template <uint16_t extension_type>
    static void delete_model_extension(AgModel& model);
};

struct AG_API AgModelHandler
{
    /**
     * @brief main Agos function to parse an .obj file with its corresponding .mtl file
     * @param model AgModel containing @b both informations to model.path_to_obj_file @a and model.path_to_texture_file
     * @param color Default color to apply to the entire model
     * @return A filled in with AgModelData struct with informations from the .obj
    */
    static AgResult     load_model          (AgModel& model, const glm::vec3& polygons_color = glm::vec3(1.0f));
    static AgResult     translate           (AgModel& model, const glm::vec3& translation);
    static AgResult     scale               (AgModel& model, const glm::vec3& translation);
    static AgResult     rotate              (AgModel& model, const glm::vec3& rotation_axis, const float& angle_degrees);
    static AgResult     set_light_source    (AgModel& model, const glm::vec3& light_color);
    static AgResult     set_lighting_map    (AgModel& model);
};
} // namespace Agos


