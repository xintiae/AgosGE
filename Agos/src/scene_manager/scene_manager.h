#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"

#include "Agos/src/entities/entities.h"
#include "Agos/src/renderer/camera/camera.h"
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
    struct SceneStatus
    {
        bool shall_draw_viewport;       // = true;

        bool scene_closed;              // = false;
        bool scene_opened;              // = false;

        bool scene_new;                 // = false;
        bool scene_save_curr;           // = false;
        bool scene_save_curr_as;        // = false;
        bool scene_open;                // = false;
        bool scene_close;               // = false;

        bool can_scene_new;             // = false;
        bool can_scene_save_curr;       // = false;
        // bool can_scene_save_curr_as;
        bool can_scene_open;            // = false;
        bool can_scene_close;           // = false;


        bool entity_new;                // = false;
        bool entity_destroy;            // = false;
        bool entity_load;               // = false;
        bool entity_unload;             // = false;
        bool entity_translate;          // = false;
        bool entity_rotate;             // = false;

        bool can_entity_new;            // = false;
        bool can_entity_destroy;        // = false;
        bool can_entity_load;           // = false;
        bool can_entity_unload;         // = false;
        bool can_entity_translate;      // = false;
        bool can_entity_rotate;         // = false;


        bool undo;                      // = false;
        bool redo;                      // = false;

        bool can_undo;                  // = false;
        bool can_redo;                  // = false;

        SceneStatus();
    };

};
// ** SceneManagerCore ==================================================================

// ** ApplicationSceneManager ===========================================================
using SceneStatus = Agos::SceneManager::SceneManagerCore::SceneStatus;

class ApplicationSceneManager
{
private:
    std::string                                             m_ScenePath;
    std::shared_ptr<SceneManagerCore::SceneStatus>          m_SceneStatus;
    std::vector<std::shared_ptr<Agos::Entities::Entity>>    m_SceneEntities;
    std::shared_ptr<Clipping::CameraObject>                 m_ViewportCamera;


    bool                                                    m_AppSceneManagerTerminated;    // = false

public:
    ApplicationSceneManager();
    ApplicationSceneManager(const ApplicationSceneManager& other)   = delete;
    ApplicationSceneManager(ApplicationSceneManager&& other)        = delete;
    ~ApplicationSceneManager();

    ApplicationSceneManager& operator=(const ApplicationSceneManager& other)    = delete;
    ApplicationSceneManager& operator=(ApplicationSceneManager&& other)         = delete;

    AgResult                init            ();
    AgResult                terminate       ();

    inline std::shared_ptr<SceneStatus>&            current_state   ()      { return m_SceneStatus;     }
    inline std::shared_ptr<Clipping::CameraObject>& viewport_camera ()      { return m_ViewportCamera;  }

private:
    AgResult    check_status();

public:
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
    const std::vector<std::shared_ptr<Entities::Entity>>&   get_scenes_entities ()                                  { return m_SceneEntities;      }
    const std::shared_ptr<Entities::Entity>&                get_entity          (const size_t& entity_id)           { return m_LookFor(entity_id);  }

    AgResult                        create_entity       (const Entities::Entity& entity_data);
    AgResult                        show_entity         (const size_t& entity_id);
    AgResult                        hide_entity         (const size_t& entity_id);
    AgResult                        set_entity_type     (const size_t& entity_id, const Entities::EntityData::Properties::EntityType& type);

    const std::vector<std::shared_ptr<Entities::Entity>>&   get_scenes_entities ()                          const   { return m_SceneEntities;      }
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
