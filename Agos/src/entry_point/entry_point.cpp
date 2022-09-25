#include "Agos/src/entry_point/entry_point.h"
#include "Agos/src/logger/logger.h"

Agos::Application::Application()
    : m_AppShouldRun    (true),
    m_AppTerminated     (false)
{
    m_AppRenderer       = std::make_unique<Agos::Renderer::ApplicationRenderer>();
    m_AppSceneManager   = std::make_unique<Agos::SceneManager::ApplicationSceneManager>();
}

Agos::Application::~Application()
{
}

Agos::AgResult Agos::Application::init()
{
    Agos::ag_init_loggers();
    AG_CORE_WARN("[Entry Point/Application - init] => Initializing AgosGE... <=");
    m_AppRenderer->init();
    m_AppSceneManager->init();
    AG_CORE_INFO("[Entry Point/Application - init] Done!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::Application::run()
{
    AG_CORE_WARN("[Entry Point/Application - run] => Running AgosGE... <=");
    m_AppSceneManager->load_scene   (AG_EXAMPLES_PATH + std::string("/scene_1-hello_agos"));
    m_AppRenderer->load_entities    (m_AppSceneManager->get_scenes_entities());
    m_AppRenderer->query_scene_state    (m_AppSceneManager->current_state());
    m_AppRenderer->set_viewport_camera  (m_AppSceneManager->viewport_camera());

    while (m_AppShouldRun)
    {
        // m_AppRenderer->draw_startup_menu();  // ImGui stuff
        // m_AppRenderer->draw_menu();          // ImGui stuff
        // m_AppRenderer->update_entities();
        // m_AppRenderer->draw_scene();

        m_AppRenderer->draw_screen();

        // performs glfwPollEvents()
        m_AppShouldRun = m_AppRenderer->app_should_run();
    }

    m_AppRenderer->unload_entities();
    m_AppSceneManager->save_scene();
    m_AppSceneManager->close_current_scene();
    return AG_SUCCESS;
}

Agos::AgResult Agos::Application::terimnate()
{
    AG_CORE_WARN("[Entry Point/Application - terminate] => Terminating AgosGE... <=");
    if (!m_AppTerminated)
    {
        m_AppSceneManager->terminate();
        m_AppRenderer->terminate();
        m_AppTerminated = true;
        return AG_SUCCESS;
    }
    AG_CORE_INFO("[Entry Point/Application - terminate] Done!");
    return AG_APPLICATION_ALREADY_TERMINATED;
}
