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

    // setup here your data for the renderer to render OR specify which models AgosGE should load by default
    // keep in mind that your models may overlap on each others

    Agos::AgModel model;
    // model.id                     = std::move(std::string("cube"));
    // model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/cube.obj"));
    // model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string(AG_DEFAULT_MODEL_TEXTURE));
    // m_Rendered_Models.push_back(std::move(model));

    model.id                     = std::move(std::string("axis"));
    model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/axis.obj"));
    model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string(AG_DEFAULT_MODEL_TEXTURE));
    m_Rendered_Models.push_back(std::move(model));

    model.id                     = std::move(std::string("teapot"));
    model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/primitives/teapot.obj"));
    model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string(AG_DEFAULT_MODEL_TEXTURE));
    m_Rendered_Models.push_back(std::move(model));

    // model.id                     = std::move(std::string("viking_room"));
    // model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/viking_room/viking_room.obj"));
    // model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string("/viking_room/viking_room.png"));
    // m_Rendered_Models.push_back(std::move(model));

    // model.id                     = std::move(std::string("arrow"));
    // model.path_to_obj_file       = std::move(std::string(AG_MODELS_PATH) + std::string("/dungeon_pack/Models/obj/wallSingle.obj"));
    // model.path_to_texture_file   = std::move(std::string(AG_MODELS_PATH) + std::string(AG_DEFAULT_MODEL_TEXTURE));
    // m_Rendered_Models.push_back(std::move(model));

    // you have to load your models' data here
    for (size_t i = 0; i < m_Rendered_Models.size(); i++)
    {
        AG_CORE_WARN("Loading model : " + m_Rendered_Models[i].id + " (obj file path : " + m_Rendered_Models[i].path_to_obj_file);
        m_Rendered_Models[i].model_data = Agos::AgModelLoader::load_model(m_Rendered_Models[i].path_to_obj_file);
        // NOTE : you can't load directly your texture here; this operation is done when initialyzing vulkan
    }

    /*** init vulkan @c WITH your models */
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
