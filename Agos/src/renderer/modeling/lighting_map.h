#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/renderer/vulkan_textures.h"

#include AG_VULKAN_INCLUDE
#include <string>

namespace Agos
{

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct AG_API AgModelDataDiffuseMap
{
    std::string                                     diffuse_path;
    std::shared_ptr<AgVulkanHandlerTextureManager>  diffuse_texture;
};

struct AG_API AgModelDataSpecularMap
{
    std::string                                     specular_texture;
    std::shared_ptr<AgVulkanHandlerTextureManager>  specular_texture;
};

// struct AG_API AgModelExtensionDataLightingMap
// is to be found in file Agos/src/renderer/modeling/model.h

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}   // namespace Agos
