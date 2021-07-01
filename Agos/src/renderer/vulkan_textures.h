#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/vulkan_logical_device.h"

#include AG_VULKAN_INCLUDE
#include <memory>
#include <vector>


namespace Agos
{

typedef class AG_API AgVulkanHandlerTextureImageManager
{
private:

    VkDevice m_LogicalDeviceReference;
    bool m_Terminated = false;

public:
    AgVulkanHandlerTextureImageManager();
    AgVulkanHandlerTextureImageManager(const VkDevice& logical_device);
    ~AgVulkanHandlerTextureImageManager();

    AgResult create_texture_image();
    AgResult terminate();

};
} // namespace Agos

