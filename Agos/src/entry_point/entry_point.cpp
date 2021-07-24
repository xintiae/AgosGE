#include "Agos/src/entry_point/entry_point.h"

#include "Agos/src/logger/logger.h"

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
     * * === THIS IS A TODO LIST === *
     * * (use Better Comments vscode extension to better visulize it :P)
     * 
     * TODO LIST
     *      ! improve color system
     *      - review .mtl files loading op
     *      ? add a material property for each model / vertex?
     *      - camera velocity vector / acceleration vector
     *
     * * LATER ON
     *      ! IMGUI MENU
     *      * Live editing & saving
     *      ? physics?
     *      - who knows what's coming next ;)
    */

    // * setup here your data for the renderer to render OR specify which models AgosGE should load by default
    // * keep in mind that your models may overlap on each others
    Agos::AgModel model;
    model.id                     = std::move(std::string("axis"));
    model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/axis.obj"));
    model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string(AG_DEFAULT_MODEL_TEXTURE));
    m_Rendered_Models.push_back(std::move(model));

    model.id                     = std::move(std::string("cube"));
    model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/cube.obj"));
    model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/cube.mtl"));
    m_Rendered_Models.push_back(std::move(model));

    model.id                     = std::move(std::string("cube"));
    model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/cube.obj"));
    model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/cube.mtl"));
    m_Rendered_Models.push_back(std::move(model));

    // * you have to load your models' data here before initializing vulkan
    for (size_t i = 0; i < m_Rendered_Models.size(); i++)
    {
        AG_CORE_WARN("Loading model : " + m_Rendered_Models[i].id + " (obj file path : " + m_Rendered_Models[i].path_to_obj_file);
        if (i == 1)
            // "wanna set ma cube blue man"
            m_Rendered_Models[i].model_data = Agos::AgModelHandler::load_model(m_Rendered_Models[i], glm::vec3(0.0f, 0.0f, 1.0f));
        else
            m_Rendered_Models[i].model_data = Agos::AgModelHandler::load_model(m_Rendered_Models[i], glm::vec3(1.0f, 1.0f, 1.0f));        
    }

    // * scale things up here
    Agos::AgModelHandler::scale(m_Rendered_Models[0], glm::vec3(0.0f));

    // * setting up a light source here
    // ! NOTE : if no model is set as a light source, the renderer provides a default light source
    // ! at location glm::vec3(1.0f, 1.0f, 1.0f) with color glm::vec3(1.0f, 0.0f, 0.0f) - which is red)
    Agos::AgModelHandler::scale(m_Rendered_Models[1], glm::vec3(0.2f));
    Agos::AgModelHandler::translate(m_Rendered_Models[1], glm::vec3(5.0f, 0.0f, 0.0f));
    Agos::AgModelHandler::set_light_source(m_Rendered_Models[1], glm::vec3(1.25f, 1.25f, 1.25f));

    // ! init vulkan WITH your loaded models
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
