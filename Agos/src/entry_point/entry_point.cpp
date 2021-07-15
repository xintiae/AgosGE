#include "Agos/src/entry_point/entry_point.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/debug_layers/debug_layers.h"
#include "Agos/src/renderer/renderer.h"

Agos::AgApplication::AgApplication()
{
    m_EventBus  = std::make_shared<dexode::EventBus>();
    m_Renderer  = std::make_shared<AgVulkanHandlerRenderer>(m_EventBus);
}

Agos::AgApplication::~AgApplication()
{
    m_Renderer->terminate();
}

Agos::AgResult Agos::AgApplication::core_init_application()
{
    Agos::ag_init_loggers();
    AG_CORE_WARN("Initializing Agos core application...");

    /**
     * ==== @b IMPORTANT ====
     * @b this is still @c WIP
     * 
     * Do @b NOT @c SETUP MORE THAN ONE MODEL TO RENDER
    */

    // setup here your data for the renderer to render
    m_Rendered_Models.resize(2);
    m_Rendered_Models[0].id                     = std::move(std::string("viking_room"));
    m_Rendered_Models[0].path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string("/viking_room/viking_room.png"));
    m_Rendered_Models[0].path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/viking_room/viking_room.obj"));

    m_Rendered_Models[1].id                     = std::move(std::string("cube"));
    m_Rendered_Models[1].path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/cube.png"));
    m_Rendered_Models[1].path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/cube.obj"));


    for (size_t i = 0; i < m_Rendered_Models.size(); i++)
    {
        AG_CORE_WARN("Loading model : " + m_Rendered_Models[i].id + " (obj file path : " + m_Rendered_Models[i].path_to_obj_file);
        m_Rendered_Models[i].model_data = Agos::AgModelLoader::load_model(m_Rendered_Models[i].path_to_obj_file);
    }

    m_Renderer->init_vulkan(m_Rendered_Models);

    AG_CORE_INFO("Done initializing Agos core application!");
    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::AgApplication::core_run_application()
{
    AG_CORE_WARN("Running Agos core application...");

    m_Renderer->run<void()>(
        std::function<void()>([&](void) -> void {})
    );

    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::AgApplication::core_terminate_application()
{
    AG_CORE_WARN("Terminating Agos core application...");

    m_Renderer->terminate_vulkan();
    m_Renderer->terminate();

    AG_CORE_INFO("Terminated Agos core application!");
    AG_CORE_WARN("Exiting...");
    return Agos::AG_SUCCESS;
}
