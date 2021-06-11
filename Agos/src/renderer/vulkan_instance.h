#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
namespace Agos{
    class AgVulkanHandlerDebugLayersManager;
}
#include "Agos/src/debug_layers/vulkan_debug_layers.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace Agos
{
typedef class AG_API AgVulkanHandlerInstance
{
private:
    VkInstance m_Instance;

public:
    AgVulkanHandlerInstance();
    ~AgVulkanHandlerInstance();

    VkInstance& get_instance();

    AgResult init_instance(const std::shared_ptr<AgVulkanHandlerDebugLayersManager>& DebugLayersManager);
} AgVulkanHandlerInstance;
} // namespace Agos
