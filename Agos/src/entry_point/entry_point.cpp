#include "Agos/src/entry_point/entry_point.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/debug_layers/debug_layers.h"
#include "Agos/src/renderer/renderer.h"

Agos::AgApplication::AgApplication()
{
    m_EventBus                  = std::make_shared<dexode::EventBus>();
    // gotta allocate mem
    m_GLFWEventsHandler         = std::make_shared<AgGLFWHandlerEvents>(m_EventBus);
    m_GLFWInstance              = std::make_shared<AgGLFWHandlerInstance>(m_EventBus);
    m_VulkanInstance            = std::make_shared<AgVulkanHandlerInstance>(m_EventBus);
    m_VulkanDebugLayersManager  = std::make_shared<AgVulkanHandlerDebugLayersManager>(m_VulkanInstance);

    m_VulkanPhysicalDevice      = std::make_shared<AgVulkanHandlerPhysicalDevice>();
}

Agos::AgApplication::~AgApplication()
{
}

Agos::AgResult Agos::AgApplication::core_init_application()
{
    Agos::ag_init_loggers();
    AG_CORE_WARN("Initializing Agos core application...");

    AG_CORE_INFO("Initializing GLFW instance...");
    m_GLFWInstance->init(m_GLFWEventsHandler);

    AG_CORE_INFO("Initializing vulkan instance...");
    m_VulkanInstance->init(m_VulkanDebugLayersManager);

    AG_CORE_INFO("Setting up debug layers...");
    m_VulkanDebugLayersManager->vulkan_setup_debug_messenger();

    AG_CORE_INFO("Picking vulkan compatible GPU...");
    m_GLFWInstance->setup_vulkan_surface(m_VulkanInstance);
    m_VulkanPhysicalDevice->pick_physical_device(m_VulkanInstance, m_GLFWInstance);


    AG_CORE_INFO("Done initializing Agos core application!");
    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::AgApplication::core_run_application()
{
    AG_CORE_WARN("Running Agos core application...");

    while ( !glfwWindowShouldClose(m_GLFWInstance->get_window()) )
    {
        glfwPollEvents();
    }


    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::AgApplication::core_terminate_application()
{
    AG_CORE_WARN("Terminating Agos core application...");

    m_GLFWInstance->terminate_vulkan_surface(m_VulkanInstance);
    m_VulkanDebugLayersManager->terminate();
    m_VulkanInstance->destroy();
    m_GLFWInstance->terminate();

    return Agos::AG_SUCCESS;
}
