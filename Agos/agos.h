#pragma once
// --- include file for AgosGE ---

#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/renderer.h"

#include "Agos/src/entry_point/entry_point.h"

/**
 * TODO LIST
 * !*!*!            CAMERA SYSTEM | implement new entity type : camera type
 *                  -> enables observing scene from different point of views 
 * -                camera move schementincs implementieren
 * // ?            move debug layers into renderer (since not in entry pt?)
 * // ?            idem but regarding event_manager
 * // !*!*!    get vulkan based graphics to work
 * // !*!*!    remove init_agosge.cpp et setup le main dans agos.cpp
 * // !*!*!    imposer lien de corrélation entre ScenesManager et Renderer au sujet des m_ToRenderEntities - bug lors de la fermeture de Agos
 * !*!*!    get proper ImGui-based gui done
 * // **           => create an offscreen opaque handler to which AgosGE's scene will be drawn; renderpass, framebuffer, graphics pipeline
 * //            !*!*!=> 2 shaders : scene's shaders AND ImGui's shaders  // NO NEED : IMGUI PROVIDES IT ALREADY
 * //             -> scene's viewport
 * *=*             -> basic scene's menu
 *              -> startup menu
 * !*!*!        -> implement ImGui-SceneManager
 *              -> entities's section
 *             ?-> file explorer
 *             ?-> dynamic text editor
 *             ?-> dynamic shaders compilation
 * !*!*!    finish up scene manager (ie at least opening, saving scenes ops and loading / saving scene's entities)
 * //             -> loading scenes
 *              -> creating scenes
 *              -> saving scenes
 *              -> closing scenes
 *              -> destroying scenes
 * //             -> AG_CORE_SPDLOG zu überprüfen
 * !*!*!        -> rework Entity's id generator
 * !*!*!        -> if an entity's missing a map, ignore its loading AND if it's got it, precise its name
 * //             !*!*!=> m_FindMapPath (cf. scene_manager.cpp)        
 * !*!          -> if an entity's primary maps aren't in its corresponding file, choose to compute them instead of loading them
 *
 * !*!*!    update_entities - finish it up - goal : unload destroyed or unshowned entities from GPU
 * !*!      Scene Manager entity creation / destruction process
 *              -> creation process
 *              -> destruction process
 * !*!      Allow for specific anti aliasing / mipmaps
 * // !*!      rework/rearrange stock entities/models in different files
 * // -        rework shaders' samplers that take in fragTextCoord instead of their corresponding coordnates
 * //             ->(ie add glm::vec2 <map_name>Coord; for instance : glm::vec2 ambtCoord; )
 * !        add alpha maps
 * -        rework Agos::Entities::Entity(const Entity& other) constructor (currently : copies)
 * -        modifier m_LookFor dans SceneManager qui retourne m_ScenesEntities[0] si l'id renseigné ne correspond à aucune entité
 * -        SceneManager : modifier la suppression d'une entité particulière de m_ScenesEntities
 * - 5      MVP_EL_LayoutBindings avec MVP accessible aussi bien vertex que fragment (cf. update_descriptor_sets)
 * ?        update_uniform_buffers : douille avec le mapping des UBOs (?)
 * 
 * TODO LATER ON
 * !*!*!    optimize buffers allocation in such a way that there's only ONE buffer for EVERY RESSOURCE
 * !*!*!    optimize command buffers reccording (slow process?)
 * !        compute shader
 * !        pipeline cache
 * -        multiple subpasses
 * -        instance rendering
 * -        push constants
 * -        familiy queues ownership for faster execution
 * -        dynamic uniforms
 */
