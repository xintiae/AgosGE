#include "Agos/src/renderer/renderer.h"

// *=* Vulkan-based Renderer ===============================================================================================
#ifdef AG_GRAPHICS_API_VULKAN

// ** RendererCore ======================================================================
Agos::Renderer::RendererCore::RendererCore()
{
    m_EventBus          = std::make_shared<dexode::EventBus>();
    m_GLFWInterface     = std::make_shared<Agos::GLFWHandler::GLFWInstance>(m_EventBus);
    m_GLFW_SIDU         = std::make_shared<Agos::GLFWHandler::Event::EventManager>(m_EventBus);
    m_VulkanRenderer    = std::make_shared<Agos::VulkanHandler::VulkanApp>(m_GLFWInterface);
}

Agos::Renderer::RendererCore::~RendererCore()
{
    terminate();
}

Agos::AgResult Agos::Renderer::RendererCore::init(
    const std::string&  window_title,
    const int&          window_width /*= AG_DEFAULT_WINDOW_WIDTH*/,
    const int&          window_height /*= AG_DEFAULT_WINDOW_HEIGHT*/
)
{
    m_GLFWInterface->init(m_GLFW_SIDU, window_title, window_width, window_height);
    m_VulkanRenderer->init_vulkan_app();

    return AG_SUCCESS;
}

Agos::AgResult Agos::Renderer::RendererCore::load_entities(
    const std::vector<std::shared_ptr<Entities::Entity>>& entities
)
{
    m_VulkanRenderer->load_entities(entities);
    return AG_SUCCESS;
}

Agos::AgResult Agos::Renderer::RendererCore::query_scene_state(
    const std::shared_ptr<Agos::SceneManager::SceneStatus>& scene_status
)
{
    m_VulkanRenderer->query_scene_state(scene_status);
    return AG_SUCCESS;
}

Agos::AgResult Agos::Renderer::RendererCore::draw()
{
    m_VulkanRenderer->draw_frame();
    return AG_SUCCESS;
}

Agos::AgBool Agos::Renderer::RendererCore::app_should_run()
{
    return m_GLFWInterface->app_should_run();
}

Agos::AgResult Agos::Renderer::RendererCore::unload_entities()
{
    m_VulkanRenderer->unload_entities();
    return AG_SUCCESS;
}

Agos::AgResult Agos::Renderer::RendererCore::terminate()
{
    m_VulkanRenderer->terminate_vulkan_app();
    m_GLFWInterface->terminate();

    return AG_SUCCESS;
}

// ** RendererCore ======================================================================


// ** ApplicationRenderer ===============================================================
Agos::Renderer::ApplicationRenderer::ApplicationRenderer()
    :
    m_AppTitle              ("AgosGE"),
    m_AppRendererTerminated (false)
{
    m_AppRenderer = std::make_unique<Agos::Renderer::RendererCore>();
}

Agos::Renderer::ApplicationRenderer::~ApplicationRenderer()
{
    terminate();
}

Agos::AgResult Agos::Renderer::ApplicationRenderer::init()
{
    m_AppRenderer->init(m_AppTitle, AG_MAX_WINDOW_WIDTH, AG_MAX_WINDOW_HEIGHT);
    return AG_SUCCESS;
}

Agos::AgResult Agos::Renderer::ApplicationRenderer::load_entities(
    const std::vector<std::shared_ptr<Entities::Entity>>& entities
)
{
    m_AppRenderer->load_entities(entities);
    return AG_SUCCESS;
}

Agos::AgResult Agos::Renderer::ApplicationRenderer::query_scene_state(
    const std::shared_ptr<Agos::SceneManager::SceneStatus>& scene_status
)
{
    m_AppRenderer->query_scene_state(scene_status);
    return AG_SUCCESS;
}

Agos::AgResult Agos::Renderer::ApplicationRenderer::draw_screen()
{
    m_AppRenderer->draw();
    return AG_SUCCESS;
}

Agos::AgBool Agos::Renderer::ApplicationRenderer::app_should_run()
{
    return m_AppRenderer->app_should_run();
}

Agos::AgResult Agos::Renderer::ApplicationRenderer::unload_entities()
{
    m_AppRenderer->unload_entities();
    return AG_SUCCESS;
}

Agos::AgResult Agos::Renderer::ApplicationRenderer::terminate()
{
    if (!m_AppRendererTerminated)
    {
        m_AppRenderer->terminate();
        m_AppRendererTerminated = true;
        return AG_SUCCESS;
    }
    return AG_RENDERER_ALREADY_TERMINATED;
}
// ** ApplicationRenderer ===============================================================

#endif
// *=* Vulkan-based Renderer ===============================================================================================
