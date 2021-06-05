#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include <vulkan/vulkan.h>

namespace Agos
{
extern AG_API VkInstance AgVulkanInstance;

AgResult AG_API ag_init_vulkan_instance();
} // namespace Agos
