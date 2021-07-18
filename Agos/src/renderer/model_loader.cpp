#include "Agos/src/renderer/model_loader.h"

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

    for (const tinyobj::shape_t& shape : shapes)
    {
        for (const tinyobj::index_t& index : shape.mesh.indices)
        {
            Agos::VulkanGraphicsPipeline::Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    AG_CORE_INFO("[ModelLoader/AgModelLoader - load_model] Successfully loaded model : " + file_path);
    return std::move(AgModelData{vertices, indices});
}
