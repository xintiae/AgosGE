#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"

#include "Agos/src/entities/entities.h"
#include <string>
#include <vector>
#include <memory>

#include AG_STB_INCLUDE

namespace Agos
{
namespace SceneManager
{
// ** SceneManagerCore ==================================================================
namespace SceneManagerCore
{
    struct ScenesEntitiesInfo
    {

    };
};
// ** SceneManagerCore ==================================================================

// ** ApplicationSceneManager ===========================================================
class ApplicationSceneManager
{
private:
    std::string                                             m_ScenePath;
    bool                                                    m_SceneClosed;                  // = true
    std::vector<std::shared_ptr<Agos::Entities::Entity>>    m_ScenesEntities;

    bool                                                    m_AppSceneManagerTerminated;    // = false

public:
    ApplicationSceneManager();
    ApplicationSceneManager(const ApplicationSceneManager& other)   = delete;
    ApplicationSceneManager(ApplicationSceneManager&& other)        = delete;
    ~ApplicationSceneManager();

    ApplicationSceneManager& operator=(const ApplicationSceneManager& other)    = delete;
    ApplicationSceneManager& operator=(ApplicationSceneManager&& other)         = delete;

    AgResult    init();
    AgResult    terminate();

    // ** = = = = = scene manip = = = = =
    AgResult    create_scene        (const std::string& path);
    AgResult    delete_scene        (const std::string& path);
    AgResult    load_scene          (const std::string& path);
    AgResult    save_scene_to_path  (const std::string& path);
    // saves curr scene to m_ScenePath
    AgResult    save_scene          ();
    AgResult    close_current_scene ();
    // ** = = = = = scene manip = = = = =

    // ** = = = = = entity misc = = = = = 
    const std::vector<std::shared_ptr<Entities::Entity>>&   get_scenes_entities ()                                  { return m_ScenesEntities;      }
    const std::shared_ptr<Entities::Entity>&                get_entity          (const size_t& entity_id)           { return m_LookFor(entity_id);  }

    AgResult                        create_entity       (const Entities::Entity& entity_data);
    AgResult                        show_entity         (const size_t& entity_id);
    AgResult                        hide_entity         (const size_t& entity_id);
    AgResult                        set_entity_type     (const size_t& entity_id, const Entities::EntityData::Properties::EntityType& type);

    const std::vector<std::shared_ptr<Entities::Entity>>&   get_scenes_entities ()                          const   { return m_ScenesEntities;      }
    const std::shared_ptr<Entities::Entity>&                get_entity          (const size_t& entity_id)   const   { return m_LookFor(entity_id);  }
    // ** = = = = = entity misc = = = = =

private:
    // ** = = = = = entity misc = = = = =
    std::shared_ptr<Entities::Entity>&          m_LookFor               (const size_t& entity_id    );
    const std::shared_ptr<Entities::Entity>&    m_LookFor               (const size_t& entity_id    )   const;
    AgResult                                    m_LoadEntityObjData     (Entities::Entity& entity   );
    AgResult                                    m_LoadEntityTextures    (Entities::Entity& entity   );

    // * = * = * = * = * = * maps miscs and utils * = * = * = * = * = *
    std::string                                 m_FindMapPath           (Entities::Entity& entity, const std::string& map_specifier);
    AgResult                                    m_FindTexturePath       (Entities::Entity& entity);
    AgResult                                    m_FindAmbiantPath       (Entities::Entity& entity);
    AgResult                                    m_FindDiffusePath       (Entities::Entity& entity);
    AgResult                                    m_FindSpecularPath      (Entities::Entity& entity);
    AgResult                                    m_FindNormalPath        (Entities::Entity& entity);
    AgResult                                    m_FindAlphaPath         (Entities::Entity& entity);
    // * = * = * = * = * = * maps miscs and utils * = * = * = * = * = *


    inline size_t                               m_GenerateNewEntityId();
    // ** = = = = = entity misc = = = = =
};
// ** ApplicationSceneManager ===========================================================

}   // namespace SceneManager (within namespace Agos)
}   // namespace Agos
