#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/scene_manager/scene_manager.h"
#include "Agos/src/renderer/renderer.h"


namespace Agos
{
class AG_API Application
{
private:
    std::unique_ptr<Agos::Renderer::ApplicationRenderer>            m_AppRenderer;      // renders Gui, Viewport
    std::unique_ptr<Agos::SceneManager::ApplicationSceneManager>    m_AppSceneManager;  // saves, loads, destroys, scenes and keeps track of each scene's entity
    bool                                                            m_AppShouldRun;     // = true;
    bool                                                            m_AppTerminated;    // = false;
    // ags_file_formater
    // ? who knows what might come next :3

public:
    Application();
    Application(const Application& other)   = delete;
    Application(Application&& other)        = delete;
    ~Application();

    Application& operator=(const Application& other)    = delete;
    Application& operator=(Application&& other)         = delete;

    AgResult    init();
    AgResult    run();
    AgResult    terimnate();
};

}
