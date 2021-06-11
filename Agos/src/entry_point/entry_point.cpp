#include "Agos/src/entry_point/entry_point.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/debug_layers/debug_layers.h"
#include "Agos/src/renderer/renderer.h"

Agos::AgApplication::AgApplication()
{
    // gotta allocate mem
    m_VulkanInstance = std::make_shared<AgVulkanHandlerInstance>();
    m_VulkanDebugLayersManager = std::make_shared<AgVulkanHandlerDebugLayersManager>();
}

Agos::AgApplication::~AgApplication()
{
}

Agos::AgResult Agos::AgApplication::core_init_application()
{
    Agos::ag_init_loggers();
    AG_CORE_WARN("Initializing Agos core application...");

    AG_CORE_INFO("Initializing vulkan instance...");
    m_VulkanInstance->init_instance(m_VulkanDebugLayersManager);

    AG_CORE_INFO("Setting up debug layers...");
    m_VulkanDebugLayersManager->vulkan_setup_debug_messenger(m_VulkanInstance);

    AG_CORE_INFO("Done initializing Agos core application!");
    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::AgApplication::core_run_application()
{
    AG_CORE_WARN("Running Agos core application...");
    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::AgApplication::core_terminate_application()
{
    AG_CORE_WARN("Terminating Agos core application...");
    return Agos::AG_SUCCESS;
}
