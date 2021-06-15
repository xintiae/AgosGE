#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
namespace Agos{
    class AgVulkanHandlerDebugLayersManager;
}
#include "Agos/src/debug_layers/vulkan_debug_layers.h"
#include <Agos/vendor/EventBus/lib/src/dexode/EventBus.hpp>

#include <vulkan/vulkan.h>
#include <memory>

namespace Agos
{
typedef class AG_API AgVulkanHandlerInstance
{
private:
    dexode::EventBus::Listener m_EventBusListener;
    VkInstance m_Instance;
    bool m_Destroyed = false;

public:
    AgVulkanHandlerInstance(const std::shared_ptr<dexode::EventBus>& event_bus);
    ~AgVulkanHandlerInstance();

    VkInstance& get_instance();

    AgResult init(const std::shared_ptr<AgVulkanHandlerDebugLayersManager>& DebugLayersManager);
    AgResult destroy();
} AgVulkanHandlerInstance;
} // namespace Agos
