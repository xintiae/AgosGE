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
typedef struct AG_API AgVertexIndexHolder
{
    std::vector<VulkanGraphicsPipeline::Vertex> vertices;
    std::vector<uint32_t> indices;
} AgVertexIndexHolder;

typedef struct AG_API AgModelLoader
{
    static AgVertexIndexHolder load_model(const std::string& file_path);
};
} // namespace Agos


