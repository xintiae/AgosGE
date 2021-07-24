#include "Agos/src/renderer/model.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/vulkan_graphics_pipeline.h"
#include <unordered_map>
#include <filesystem>

Agos::AgModel::AgModel()
{
}

Agos::AgModel::~AgModel()
{
    if (this->extension_type == Agos::AgModelExtensionDataType::light_source)
    {
        Agos::AgModelExtensionDataLight* extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(this->pExtensionData);
        delete (extension_data);
        this->extension_type = Agos::AgModelExtensionDataType::none;
    }
}

Agos::AgModelData Agos::AgModelHandler::load_model(AgModel& model, const glm::vec3& polygons_color)
{
    bool has_mtl = false;
    tinyobj::ObjReader reader;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;


    if (model.path_to_texture_file.substr(model.path_to_texture_file.size() - 3, 3) == "mtl")
    {
        has_mtl = true;

        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = model.path_to_texture_file;
        model.path_to_texture_file = std::move(std::string(AG_MODELS_PATH) + std::string(AG_DEFAULT_MODEL_TEXTURE));

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
            return std::move(Agos::AgModelData{});
        }

        attrib      = std::move( reader.GetAttrib()     );
        shapes      = std::move( reader.GetShapes()     );
        materials   = std::move( reader.GetMaterials()  );
    }
    else
    {
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model.path_to_obj_file.c_str()))
        {
            AG_CORE_ERROR("[/AgModelHandler - load_model] " + warn + err);
            return std::move(Agos::AgModelData{});
        }
    }

    std::vector<VulkanGraphicsPipeline::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Agos::VulkanGraphicsPipeline::Vertex, uint32_t> uniqueVertices{};
    bool normals_included = false;

    for (const tinyobj::shape_t& shape : shapes)
    {
        for (const tinyobj::index_t& index : shape.mesh.indices)
        {
            Agos::VulkanGraphicsPipeline::Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            if (index.texcoord_index >= 0 && !has_mtl)
            {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
            }

            if (attrib.colors.size() != 0)
            {
                tinyobj::real_t red   = attrib.colors[3 * (index.vertex_index) + 0];
                tinyobj::real_t green = attrib.colors[3 * (index.vertex_index) + 1];
                tinyobj::real_t blue  = attrib.colors[3 * (index.vertex_index) + 2];
                vertex.color = std::move(glm::vec3{red, green, blue} * polygons_color);
            }

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

    AG_CORE_INFO("[ModelLoader/AgModelHandler - load_model] Successfully loaded model : " + model.path_to_obj_file);
    return std::move(AgModelData{vertices, indices, glm::vec3(0.0f)});
}

void Agos::AgModelHandler::translate(AgModel& model, const glm::vec3& translation)
{
    for (Agos::VulkanGraphicsPipeline::Vertex& vertex : model.model_data.vertices)
    {
        vertex.pos += translation;
    }
    model.model_data.translation = std::move(translation);
}

void Agos::AgModelHandler::scale(AgModel& model, const glm::vec3& translation)
{
    glm::mat4 m(1.0f);  // 'm' stands for "model"
    m = glm::scale(m, translation);
    for (Agos::VulkanGraphicsPipeline::Vertex& vertex : model.model_data.vertices)
    {
        vertex.pos = m * glm::vec4(vertex.pos, 1.0f);
    }
}

void Agos::AgModelHandler::set_light_source(AgModel& model, const glm::vec3& light_color)
{
    model.extension_type = std::move(Agos::AgModelExtensionDataType::light_source);
    model.pExtensionData = new (Agos::AgModelExtensionDataLight);
    Agos::AgModelExtensionDataLight* extension_data = reinterpret_cast<Agos::AgModelExtensionDataLight*>(model.pExtensionData);
    extension_data->light_position = model.model_data.vertices[0].pos;
    extension_data->light_color = light_color;
}
