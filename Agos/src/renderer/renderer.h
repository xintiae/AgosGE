#pragma once


#ifdef AG_GRAPHICS_API_VULKAN
#include "Agos/src/renderer/vulkan_app/vulkan_app.h"
#endif

#ifdef AG_GRAPHICS_API_OPENGL
#include "Agos/src/renderer/opengl_app/opengl_app.h"
#endif

#include AG_EVENTBUS_INCLUDE
#include "Agos/src/renderer/glfw_instance.h"
#include "Agos/src/base.h"
#include "Agos/src/core.h"


namespace Agos
{

namespace Renderer
{
// ** RendererCore ======================================================================
#ifdef AG_GRAPHICS_API_VULKAN

class AG_API RendererCore
{
private:
    std::shared_ptr<dexode::EventBus>                       m_EventBus;
    std::shared_ptr<Agos::GLFWHandler::GLFWInstance>        m_GLFWInterface;
    std::shared_ptr<Agos::GLFWHandler::Event::EventManager> m_GLFW_SIDU;

    std::shared_ptr<Agos::VulkanHandler::VulkanApp>         m_VulkanRenderer;

public:
    RendererCore();
    RendererCore(const RendererCore& other) = delete;
    RendererCore(RendererCore&& other)      = delete;
    ~RendererCore();

    RendererCore& operator=(const RendererCore& other)  = delete;
    RendererCore& operator=(RendererCore&& other)       = delete;

    AgResult    init            (const std::string& window_title, const int& window_width = AG_DEFAULT_WINDOW_WIDTH, const int& window_height = AG_DEFAULT_WINDOW_HEIGHT);
    AgResult    load_entities   (const std::vector<std::shared_ptr<Entities::Entity>>& entities);
    AgResult    update_entities ();
    AgResult    draw            ();
    AgBool      app_should_run  ();
    AgResult    unload_entities ();
    AgResult    terminate       ();

private:
    //  draw_screen()
    //  {
    AgResult    draw_gui();
    AgResult    draw_viewport();
    //  }
};

#elif defined AG_GRAPHICS_API_OPENGL
//    OpenGLHandler::OpenGLApp            m_OpenGLRenderer;

#endif
// ** RendererCore ======================================================================

// ** ApplicationRenderer ===============================================================
class AG_API ApplicationRenderer
{
private:
    std::unique_ptr<RendererCore>       m_AppRenderer;
    std::string                         m_AppTitle;                 // = "AgosGE" or "Scene Editor - Powered by AgosGE"
    bool                                m_AppRendererTerminated;    // = false by default

public:
    explicit ApplicationRenderer();
    explicit ApplicationRenderer(const ApplicationRenderer& other)    = delete;
    explicit ApplicationRenderer(ApplicationRenderer&& other)         = delete;

    virtual ~ApplicationRenderer();

    ApplicationRenderer& operator=(const ApplicationRenderer& other)  = delete;
    ApplicationRenderer& operator=(ApplicationRenderer&& other)       = delete;

    AgResult    init            ();
    // loads the specified entities onto the GPU for drawing
    AgResult    load_entities   (const std::vector<std::shared_ptr<Entities::Entity>>& entities);
    // checks for valid entities to be drawn (i.e. if an entity's signatured as destroyed, unload it from gpu and proceed to drawning)
    AgResult    update_entities ();
    AgResult    draw_scene      ();
    AgBool      app_should_run  ();
    AgResult    unload_entities ();
    AgResult    terminate       ();
};  // class Renderer (within namespace Agos)
// ** ApplicationRenderer ===============================================================
}   // namespace Renderer (within namespace Agos)
}   // namespace Agos
