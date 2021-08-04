#include "Agos/src/entry_point/entry_point.h"

#include "Agos/src/logger/logger.h"
#include <chrono>

Agos::AgApplication::AgApplication()
{
    m_EventBus          = std::make_shared<dexode::EventBus>();
    m_Renderer          = std::make_shared<AgVulkanHandlerRenderer>(m_EventBus);
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
     *      - material property for each model
     *      - review .mtl files loading op
     *      - camera velocity vector / acceleration vector
     *      ! IMGUI MENU
     *
     * * LATER ON
     *      - Live editing & saving
     *      - physics?
     *      - who knows what's coming next ;)
    */

    // * setup here your data for the renderer to render OR specify which models AgosGE should load by default
    // * keep in mind that your models may overlap on each others when rendering your scene without translating
    m_Rendered_Models.reserve(3);
    Agos::AgModel model;
    model.id                     = std::move(std::string("axis"));
    model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/axis.obj"));
    model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string(AG_DEFAULT_MODEL_TEXTURE));
    m_Rendered_Models.push_back(std::move(model));

    model.id                     = std::move(std::string("cube"));
    model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/cube.obj"));
    model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string(AG_DEFAULT_MODEL_TEXTURE));
    m_Rendered_Models.push_back(std::move(model));

    model.id                     = std::move(std::string("teapot"));
    model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/teapot.obj"));
    model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string(AG_DEFAULT_MODEL_TEXTURE));
    m_Rendered_Models.push_back(std::move(model));

    // model.id                     = std::move(std::string("wallSingle.obj"));
    // model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/dungeon_pack/Models/obj/wallSingle.obj"));
    // model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string(AG_DEFAULT_MODEL_TEXTURE));
    // m_Rendered_Models.push_back(std::move(model));


    // * you have to load your models' data here before initializing vulkan
    for (size_t i = 0; i < m_Rendered_Models.size(); i++)
    {
        AG_CORE_WARN("Loading model : " + m_Rendered_Models[i].id + " (obj file path : " + m_Rendered_Models[i].path_to_obj_file + ")");
        if (i == 1)
            // "wanna set ma cube blue man"
            Agos::AgModelHandler::load_model(m_Rendered_Models[i], glm::vec3(0.0f, 0.0f, 1.0f));
        else if (i == 2)
            // "wanna set ma teapot green man"
            Agos::AgModelHandler::load_model(m_Rendered_Models[i], glm::vec3(0.0f, 1.0f, 0.0f));
        else
            // "wanna set it any color?" - "nah I'm fine with white this time"
            Agos::AgModelHandler::load_model(m_Rendered_Models[i], glm::vec3(1.0f, 1.0f, 1.0f));        
    }


    // * scale things up here
    Agos::AgModelHandler::scale(m_Rendered_Models[0], glm::vec3(0.0f));


    // * setting up a light source here
    // NOTE : if no model is set as a light source, the renderer provides a default light source
    // at location glm::vec3(1.0f, 1.0f, 1.0f) with color glm::vec3(1.0f, 0.0f, 0.0f) - which is red
    Agos::AgModelHandler::scale(m_Rendered_Models[1], glm::vec3(0.2f));
    Agos::AgModelHandler::translate(m_Rendered_Models[1], glm::vec3(5.0f, 0.0f, 0.0f));
    Agos::AgModelHandler::set_light_source(m_Rendered_Models[1], glm::vec3(1.25f, 1.25f, 1.25f));

    bool should_cursor_exist = true;
    // ! init vulkan WITH your loaded models
    m_Renderer->init_vulkan(m_Rendered_Models, should_cursor_exist);

    AG_CORE_INFO("Done initializing Agos core application!");
    return Agos::AG_SUCCESS;
}

Agos::AgResult Agos::AgApplication::core_run_application()
{
    AG_CORE_WARN("Running Agos core application...");

    while (m_Renderer->can_run())
    {
        std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();

        std::vector<Agos::AgModel> updated_models(1, m_Rendered_Models[1]);
        // ! !=!=!=! NOTE : UNDER ANY CIRCUMSTANCES DO NOT TRY TO RESIZE ANY m_Rendered_Models.model_data !=!=!=!
        m_Renderer->update_models_data( updated_models );
        m_Renderer->run();

        std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

        Agos::AgModelHandler::rotate(m_Rendered_Models[1], glm::vec3(0.0f, 1.0f, 1.0f), deltaTime * 20.0f);
    }

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
