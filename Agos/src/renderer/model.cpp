#include "Agos/src/renderer/model.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/vulkan_graphics_pipeline.h"
#include <unordered_map>


Agos::AgModelData Agos::AgModelLoader::load_model(const std::string& file_path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_path.c_str()))
    {
        AG_CORE_ERROR("[ModelLoader/AgModelLoader - load_model] " + warn + err);
        return std::move(Agos::AgModelData{});
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

            if (index.texcoord_index != -1)
            {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
            }

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (index.normal_index != -1)
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
                // vertices[indices[vertex - 1]],  // p1
                // vertices[indices[vertex    ]],  // p2
                // vertices[indices[vertex + 1]]   // p3

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

    AG_CORE_INFO("[ModelLoader/AgModelLoader - load_model] Successfully loaded model : " + file_path);
    return std::move(AgModelData{vertices, indices});
}
