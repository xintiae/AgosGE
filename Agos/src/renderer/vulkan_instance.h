#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
namespace Agos{
    class AgVulkanHandlerDebugLayersManager;
}
#include "Agos/src/debug_layers/vulkan_debug_layers.h"

#include AG_EVENTBUS_INCLUDE
#include AG_VULKAN_INCLUDE
#include <memory>

namespace Agos
{
class AG_API AgVulkanHandlerInstance
{
private:
    dexode::EventBus::Listener m_EventBusListener;
    VkInstance m_Instance;
    bool m_Destroyed = false;

public:
    AgVulkanHandlerInstance(const std::shared_ptr<dexode::EventBus>& event_bus);
    ~AgVulkanHandlerInstance();

    AgVulkanHandlerInstance(const AgVulkanHandlerInstance& other)   = delete;
    AgVulkanHandlerInstance(AgVulkanHandlerInstance&& other)        = delete;

    VkInstance& get_instance();

    AgResult init(const std::shared_ptr<AgVulkanHandlerDebugLayersManager>& DebugLayersManager);
    AgResult destroy();
};  // class AgVulkanHandlerInstance
} // namespace Agos
