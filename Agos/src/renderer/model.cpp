#include "Agos/src/renderer/model.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/vulkan_graphics_pipeline.h"
#include <unordered_map>
#include <filesystem>

Agos::AgModel::AgModel()
{
}

Agos::AgModel::AgModel(const AgModel& other)
{
    this->id                    = other.id;
    this->path_to_obj_file      = other.path_to_obj_file;
    this->path_to_texture_file  = other.path_to_texture_file;
    this->model_data            = other.model_data;
    this->extension_type        = other.extension_type;

    if (other.extension_type == Agos::AgModelExtensionDataType::light_source)
    {
        this->pExtensionData = new (Agos::AgModelExtensionDataLight);
        Agos::AgModelExtensionDataLight* others_extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(other.pExtensionData);
        Agos::AgModelExtensionDataLight* extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(this->pExtensionData);
        extension_data->light_position = others_extension_data->light_position;
        extension_data->light_color = others_extension_data->light_color;
    }
}

Agos::AgModel::AgModel(AgModel&& other)
{
    this->id                    = std::move(other.id);
    this->path_to_obj_file      = std::move(other.path_to_obj_file);
    this->path_to_texture_file  = std::move(other.path_to_texture_file);
    this->model_data            = std::move(other.model_data);

    this->extension_type        = std::move(other.extension_type);
    this->pExtensionData        = other.pExtensionData;

    other.extension_type = Agos::AgModelExtensionDataType::none;
    other.pExtensionData = NULL;
}


Agos::AgModel::~AgModel()
{
    if (this->extension_type == Agos::AgModelExtensionDataType::light_source)
    {
        this->extension_type = Agos::AgModelExtensionDataType::none;
        Agos::AgModelExtensionDataLight* extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(this->pExtensionData);
        delete (extension_data);
        this->pExtensionData = NULL;
    }
}

Agos::AgModel& Agos::AgModel::operator=(const Agos::AgModel& other)
{
    this->id                    = other.id;
    this->path_to_obj_file      = other.path_to_obj_file;
    this->path_to_texture_file  = other.path_to_texture_file;
    this->model_data            = other.model_data;
    this->extension_type        = other.extension_type;

    if (other.extension_type == Agos::AgModelExtensionDataType::light_source)
    {
        this->pExtensionData = new (Agos::AgModelExtensionDataLight);
        Agos::AgModelExtensionDataLight* others_extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(other.pExtensionData);
        Agos::AgModelExtensionDataLight* extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(this->pExtensionData);
        extension_data->light_position = others_extension_data->light_position;
        extension_data->light_color = others_extension_data->light_color;
    }
    return *this;
}

Agos::AgModel& Agos::AgModel::operator=(Agos::AgModel&& other)
{
    this->id                    = std::move(other.id);
    this->path_to_obj_file      = std::move(other.path_to_obj_file);
    this->path_to_texture_file  = std::move(other.path_to_texture_file);
    this->model_data            = std::move(other.model_data);

    this->extension_type        = std::move(other.extension_type);
    this->pExtensionData        = other.pExtensionData;

    other.extension_type = Agos::AgModelExtensionDataType::none;
    other.pExtensionData = NULL;

    return *this;
}

Agos::AgResult Agos::AgModelHandler::load_model(AgModel& model, const glm::vec3& polygons_color)
{
    bool has_mtl = false;

    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    reader_config.mtl_search_path = "";

    if (!reader.ParseFromFile(model.path_to_obj_file, reader_config))
    {
        if (!reader.Error().empty())
        {
            AG_CORE_ERROR("[ModelLoader/AgModelHandler - load_model] TinyObjReader : " + reader.Error());
        }
        if (!reader.Warning().empty())
        {
            AG_CORE_ERROR("[ModelLoader/AgModelHandler - load_model] TinyObjReader : " + reader.Warning());
        }
        return AG_FAILED_TO_READ_OBJ_FILE;
    }
    if (!reader.Warning().empty())
        AG_CORE_WARN("[ModelLoader/AgModelHandler - load_model] TinyObjReader : " + reader.Warning());

    attrib      = std::move( reader.GetAttrib()     );
    shapes      = std::move( reader.GetShapes()     );
    materials   = std::move( reader.GetMaterials()  );

    if (!materials.empty())
        has_mtl = true;

    std::vector<VulkanGraphicsPipeline::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Agos::VulkanGraphicsPipeline::Vertex, uint32_t> uniqueVertices{};
    bool normals_included = false;

    for (const tinyobj::shape_t& shape : shapes)
    {
        for (const tinyobj::index_t& index : shape.mesh.indices)
        {
            Agos::VulkanGraphicsPipeline::Vertex vertex{};

            // position
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            // texture
            if (index.texcoord_index >= 0 && !has_mtl)
            {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
            }

            // color
            if (attrib.colors.size() != 0)
            {
                tinyobj::real_t red   = attrib.colors[3 * (index.vertex_index) + 0];
                tinyobj::real_t green = attrib.colors[3 * (index.vertex_index) + 1];
                tinyobj::real_t blue  = attrib.colors[3 * (index.vertex_index) + 2];
                vertex.color = std::move(glm::vec3{red, green, blue} * polygons_color);
            }
            else
            {
                vertex.color = polygons_color;
            }
            
            // normals
            if (index.normal_index >= 0)
            {
                normals_included = true;
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    if (!normals_included)
    {
        for (size_t vertex = 1; vertex < indices.size(); vertex += 3)
        {
            glm::vec3 p1_p2 {
                vertices[indices[vertex]].pos.x - vertices[indices[vertex - 1]].pos.x,
                vertices[indices[vertex]].pos.y - vertices[indices[vertex - 1]].pos.y,
                vertices[indices[vertex]].pos.z - vertices[indices[vertex - 1]].pos.z
            };
            glm::vec3 p1_p3 {
                vertices[indices[vertex + 1]].pos.x - vertices[indices[vertex - 1]].pos.x,
                vertices[indices[vertex + 1]].pos.y - vertices[indices[vertex - 1]].pos.y,
                vertices[indices[vertex + 1]].pos.z - vertices[indices[vertex - 1]].pos.z
            };
            glm::vec3 normal = glm::cross(p1_p2, p1_p3);
            vertices[indices[vertex - 1]].normal = normal;
            vertices[indices[vertex]].normal = normal;
            vertices[indices[vertex + 1]].normal = normal;
        }
    }

    model.model_data = std::move(AgModelData{vertices, indices, glm::vec3(0.0f)});

    if (has_mtl)
    {
        model.model_data.materials.resize(shapes.size());
        for (size_t i = 0; i < shapes.size(); i++)
        {
            Agos::AgModelDataMaterial material;
            material.ambient.x     = materials[i].ambient[0];
            material.ambient.y     = materials[i].ambient[1];
            material.ambient.z     = materials[i].ambient[2];

            material.diffuse.x     = materials[i].diffuse[0];
            material.diffuse.y     = materials[i].diffuse[1];
            material.diffuse.z     = materials[i].diffuse[2];

            material.specular.x    = materials[i].specular[0];
            material.specular.y    = materials[i].specular[1];
            material.specular.z    = materials[i].specular[2];

            material.shininess     = materials[i].shininess;
            model.model_data.materials[i] = std::move(material);
        }
    }
    else
    {
        model.model_data.materials.resize(1);
        model.model_data.materials[0].ambient.x     = 0.25f;
        model.model_data.materials[0].ambient.y     = 0.25f;
        model.model_data.materials[0].ambient.z     = 0.25f;

        model.model_data.materials[0].diffuse.x     = 0.25f;
        model.model_data.materials[0].diffuse.y     = 0.25f;
        model.model_data.materials[0].diffuse.z     = 0.25f;

        model.model_data.materials[0].specular.x    = 0.5f;
        model.model_data.materials[0].specular.y    = 0.5f;
        model.model_data.materials[0].specular.z    = 0.5f;

        model.model_data.materials[0].shininess     = 1.0f;
    }

    AG_CORE_INFO("[ModelLoader/AgModelHandler - load_model] Successfully loaded model : " + model.path_to_obj_file);
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgModelHandler::translate(AgModel& model, const glm::vec3& translation)
{
    for (Agos::VulkanGraphicsPipeline::Vertex& vertex : model.model_data.vertices)
    {
        vertex.pos += translation;
    }
    model.model_data.translation = std::move(translation);
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgModelHandler::scale(AgModel& model, const glm::vec3& translation)
{
    glm::mat4 m(1.0f);  // 'm' stands for "model"
    m = glm::scale(m, translation);
    for (Agos::VulkanGraphicsPipeline::Vertex& vertex : model.model_data.vertices)
    {
        vertex.pos = m * glm::vec4(vertex.pos, 1.0f);
    }

    // non-uniform scaling
    if (translation.x != translation.y || translation.y != translation.z || translation.z != translation.x)
    {
        for (Agos::VulkanGraphicsPipeline::Vertex& vertex : model.model_data.vertices)
        {
            vertex.pos = glm::transpose(glm::inverse(m)) * glm::vec4(vertex.pos, 1.0f); 
        }
    }
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgModelHandler::rotate(AgModel& model, const glm::vec3& rotation_axis, const float& angle_degrees)
{
    glm::mat4 m(1.0f);  // 'm' stands for "model"
    m = glm::rotate(m, glm::radians(angle_degrees), rotation_axis);

    for (Agos::VulkanGraphicsPipeline::Vertex& vertex : model.model_data.vertices)
    {
        vertex.pos = glm::vec3(m * glm::vec4(vertex.pos, 1.0f));// + model.model_data.translation;
    }

    if (model.extension_type == Agos::AgModelExtensionDataType::light_source)
    {
        Agos::AgModelExtensionDataLight* extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(model.pExtensionData);
        extension_data->light_position = model.model_data.vertices[0].pos;
    }

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgModelHandler::set_light_source(AgModel& model, const glm::vec3& light_color)
{
    model.extension_type = std::move(Agos::AgModelExtensionDataType::light_source);
    model.pExtensionData = new (Agos::AgModelExtensionDataLight);
    Agos::AgModelExtensionDataLight* extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(model.pExtensionData);
    extension_data->light_position = model.model_data.vertices[0].pos;
    extension_data->light_color = light_color;

    return AG_SUCCESS;
}
