#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_graphics_pipeline.h"

#include <string>
#include <vector>
#include <tuple>
#include <cstdint>

#include AG_TINY_OBJ_LOADER_INCLUDE

namespace Agos
{
typedef struct AG_API AgModelData
{
    std::vector<VulkanGraphicsPipeline::Vertex> vertices;
    std::vector<uint32_t> indices;
} AgModelData;

typedef struct AG_API AgModel
{
    std::string id;
    std::string path_to_obj_file;
    std::string path_to_texture_file;
    AgModelData model_data;
} AgModel;

typedef struct AG_API AgModelLoader
{
    static AgModelData load_model(const std::string& file_path);
};
} // namespace Agos


