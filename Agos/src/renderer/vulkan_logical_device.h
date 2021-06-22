#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/debug_layers/vulkan_debug_layers.h"
#include "Agos/src/renderer/glfw_instance.h"
#include "Agos/src/renderer/vulkan_instance.h"
namespace Agos{
    class AgVulkanHandlerPhysicalDevice;
}
#include "Agos/src/renderer/vulkan_physical_device.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace Agos
{

typedef class AG_API AgVulkanHandlerLogicalDevice
{
private:
    VkDevice m_LogicalDevice;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    bool m_Terminated = false;

public:
    AgVulkanHandlerLogicalDevice();
    ~AgVulkanHandlerLogicalDevice();

    AgResult create_logical_device(
        const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance,
        const std::shared_ptr<AgVulkanHandlerDebugLayersManager>& debug_layers_manager);
    AgResult terminate();

    VkDevice& get_device();

} AgVulkanHandlerLogicalDevice;

}   // namespace Agos
