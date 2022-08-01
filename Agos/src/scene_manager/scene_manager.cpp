#include "Agos/src/scene_manager/scene_manager.h"

#include "Agos/src/logger/logger.h"
#include AG_TINY_OBJ_LOADER_INCLUDE
#include AG_JSON_INCLUDE
#include <filesystem>
#include <fstream>

// !*!*! FOR DEV PURPOSES ONLY - DO NOT CONSIDER USING THIS IN ANY OTHER CONTEXT ================
#ifdef AG_GRAPHICS_API_VULKAN
    #include "Agos/src/renderer/vulkan_app/vulkan_entity.h"
    using namespace Agos::VulkanHandler::VulkanEntity;
#endif
#ifdef AG_GRAPHICS_API_OPENGL
    #include "Agos/src/renderer/opengl_app/opengl_entity.h"
    // using namespace Agos::OpenGLHandler::OpenGLEntity;
#endif
// !*!*! FOR DEV PURPOSES ONLY - DO NOT CONSIDER USING THIS IN ANY OTHER CONTEXT ================



// ** AgosGE ApplicationSceneManager class ==========================================================================
// * = = = = = = = = = = = = = = = = = = = = constructor, destructor = = = = = = = = = = = = = = = = = = = =
Agos::SceneManager::ApplicationSceneManager::ApplicationSceneManager()
    : m_SceneClosed             (true),
    m_AppSceneManagerTerminated (false)
{
}

Agos::SceneManager::ApplicationSceneManager::~ApplicationSceneManager()
{
    terminate();
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::init()
{
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::terminate()
{
    if (!m_AppSceneManagerTerminated)
    {
        for (const std::shared_ptr<Agos::Entities::Entity>& it : m_ScenesEntities)
        {
            it->destroy_entity();
        }
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = constructor, destructor = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = scene manip = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::SceneManager::ApplicationSceneManager::create_scene(
    const std::string& scene_path
)
{
    AG_CORE_WARN("[Create Scene/SceneManager::ApplicationSceneManager - create_scene] Creating new scene at path : \"" + scene_path + "\"...");
    // create scene's directory
    std::filesystem::create_directories(scene_path);
    // create scene.ags
	std::ofstream(scene_path + "/scene.ags");

    // creating a scene assumes the scene's EMPTY - no entity to save
    // so we're straight on creating an empty entities.json
    std::filesystem::create_directories(scene_path + "/Entities");
    std::ofstream(scene_path + "/entities.json");

    m_ScenePath     = scene_path;
    m_SceneClosed   = false;
    AG_CORE_INFO("[Create Scene/SceneManager::ApplicationSceneManager - create_scene] Successfully created scene at path : \"" + scene_path + "\"!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::delete_scene(
    const std::string& scene_path
)
{
    AG_CORE_WARN("[Delete Scene/SceneManager::ApplicationSceneManager - delete_scene] Deleting current scene (scene's path : \'" + scene_path + "\')...");
    std::filesystem::remove_all(scene_path);
    m_ScenePath     = AG_SCENE_PATH_NONE;
    m_SceneClosed   = true;
    for (const std::shared_ptr<Agos::Entities::Entity>& it: m_ScenesEntities)
    {
        it->destroy_entity();
    }
    AG_CORE_INFO("[Delete Scene/SceneManager::ApplicationSceneManager - delete_scene] Deleted scene located at path : \"" + scene_path + "\"!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::load_scene(
    const std::string& scene_path
)
{
    AG_CORE_WARN("[Load Scene/SceneManager::ApplicationSceneManager - load_scene] Loading AgosGE's scene from path : \'" + scene_path + "\'...");
	std::filesystem::directory_entry scenes_entry(scene_path + "/scene.ags");
	if (scenes_entry.exists())
	{
        AG_CORE_INFO("[Load Scene/SceneManager::ApplicationSceneManager - load_scene] Found valid AgosGE's scene at path : \'" + scene_path + "\'!");

        // entities nb
        size_t entities_nb = 0;
		for (const std::filesystem::directory_entry& it : std::filesystem::directory_iterator(scene_path + std::string("/Entities")))
		{
            entities_nb += 1;
            AG_MARK_AS_USED(it);
		}
        m_ScenesEntities.resize(entities_nb);
        for (std::shared_ptr<Agos::Entities::Entity>& it : m_ScenesEntities)
        {
            it = std::make_shared<Agos::Entities::Entity>();
        }

        entities_nb = 0;
		for (const std::filesystem::directory_entry& it : std::filesystem::directory_iterator(scene_path + std::string("/Entities")))
		{
            m_ScenesEntities[entities_nb]->get_entity_data().entity_name    = it.path().stem().c_str();
            m_ScenesEntities[entities_nb]->get_entity_id()                  = this->m_GenerateNewEntityId();
            m_ScenesEntities[entities_nb]->get_entity_gpu_status()          = false;
            m_ScenesEntities[entities_nb]->should_be_shown()                = true;

            // ${SCENE_PATH}/Entities/${ENTITY_NAME}/${ENTITY_NAME}.obj
            m_ScenesEntities[entities_nb]->get_entity_data().obj_file_path =
                scene_path + "/Entities/" + m_ScenesEntities[entities_nb]->get_entity_data().entity_name + "/" +
                m_ScenesEntities[entities_nb]->get_entity_data().entity_name + ".obj";
            entities_nb++;
		}

        
        // read entities.json and fill in corresponding data
        std::ifstream   e_manifest_stream(scene_path + std::string("/entities.json"));
        std::string     e_manifest_contents(std::istreambuf_iterator<char>(e_manifest_stream), std::istreambuf_iterator<char>()); 
        // e_manifest
        nlohmann::json  entities = nlohmann::json::parse(e_manifest_stream);

        for (std::shared_ptr<Agos::Entities::Entity>& it : m_ScenesEntities)
        {
            it->get_entity_data().translation.x             = entities["Entities"][it->get_entity_data().entity_name]["translation"]["x"];
            it->get_entity_data().translation.y             = entities["Entities"][it->get_entity_data().entity_name]["translation"]["y"];
            it->get_entity_data().translation.z             = entities["Entities"][it->get_entity_data().entity_name]["translation"]["z"];

            it->get_entity_data().rotation.x                = entities["Entities"][it->get_entity_data().entity_name]["rotation"]["x"];
            it->get_entity_data().rotation.y                = entities["Entities"][it->get_entity_data().entity_name]["rotation"]["y"];
            it->get_entity_data().rotation.z                = entities["Entities"][it->get_entity_data().entity_name]["rotation"]["z"];

            it->get_entity_data().overall_color.r           = entities["Entities"][it->get_entity_data().entity_name]["overall_color"]["r"];
            it->get_entity_data().overall_color.g           = entities["Entities"][it->get_entity_data().entity_name]["overall_color"]["g"];
            it->get_entity_data().overall_color.b           = entities["Entities"][it->get_entity_data().entity_name]["overall_color"]["b"];
            it->get_entity_data().overall_color.a           = entities["Entities"][it->get_entity_data().entity_name]["overall_color"]["a"];


            // it->get_entity_data().materials.has_mtl         = entities["Entities"][it->get_entity_data().entity_name]["materials"]["has_mlt"];
            it->get_entity_data().materials.has_texture     = entities["Entities"][it->get_entity_data().entity_name]["materials"]["has_texture"];
            it->get_entity_data().materials.has_ambiant     = entities["Entities"][it->get_entity_data().entity_name]["materials"]["has_ambiant"];
            it->get_entity_data().materials.has_diffuse     = entities["Entities"][it->get_entity_data().entity_name]["materials"]["has_diffuse"];
            it->get_entity_data().materials.has_specular    = entities["Entities"][it->get_entity_data().entity_name]["materials"]["has_specular"];
            it->get_entity_data().materials.has_normal      = entities["Entities"][it->get_entity_data().entity_name]["materials"]["has_normal"];
            // it->get_entity_data().materials.has_alpha       = entities["Entities"][it->get_entity_data().entity_name]["materials"]["has_alpha"];s

            it->get_entity_data().materials.shininess       = entities["Entities"][it->get_entity_data().entity_name]["materials"]["shininess"];
            it->get_entity_data().materials.ior             = entities["Entities"][it->get_entity_data().entity_name]["materials"]["ior"];
            it->get_entity_data().materials.opacity         = entities["Entities"][it->get_entity_data().entity_name]["materials"]["opacity"];

            
            it->get_entity_data().properties.physical_body.mass     = entities["Entities"][it->get_entity_data().entity_name]["properties"]["physical_body"]["mass"];
            it->get_entity_data().properties.physical_body.charge   = entities["Entities"][it->get_entity_data().entity_name]["properties"]["physical_body"]["charge"];
            AG_CORE_INFO("[Load Scene/SceneManager::ApplicationSceneManager - load_scene] Loaded entity : \'" + it->get_entity_data().entity_name + "\'!");
        }
        e_manifest_stream.close();

        m_ScenePath     = scene_path;
        m_SceneClosed   = false;

        // load each entity's vertex / index / maps data
        for (const std::shared_ptr<Agos::Entities::Entity>& it : m_ScenesEntities)
        {
            m_LoadEntityObjData(*it.get());
            m_LoadEntityTextures(*it.get());
        }

        AG_CORE_INFO("[Load Scene/SceneManager::ApplicationSceneManager - load_scene] Successfully loaded AgosGE's scene from path : \'" + scene_path + "\'!");
		return AG_SUCCESS;
	}
    AG_CORE_ERROR("[Load Scene/SceneManager::ApplicationSceneManager - load_scene] Couldn't find any valid AgosGE's scene at path : \'" + scene_path + "\'!");
	return AG_FAILED;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::save_scene_to_path(const std::string& scene_path)
{
    AG_CORE_INFO("[Save Scene/SceneManager::ApplicationSceneManager - save_scene] Saving current scene to : \'" + scene_path + "\'...");
	nlohmann::json json_contents;
	for (const std::shared_ptr<Agos::Entities::Entity>& it : m_ScenesEntities)
	{
        json_contents["Entities"][it->get_entity_data().entity_name]["translation"]["x"]            = it->get_entity_data().translation.x;
        json_contents["Entities"][it->get_entity_data().entity_name]["translation"]["y"]            = it->get_entity_data().translation.y;
        json_contents["Entities"][it->get_entity_data().entity_name]["translation"]["z"]            = it->get_entity_data().translation.z;

        json_contents["Entities"][it->get_entity_data().entity_name]["rotation"]["x"]               = it->get_entity_data().rotation.x;
        json_contents["Entities"][it->get_entity_data().entity_name]["rotation"]["y"]               = it->get_entity_data().rotation.y;
        json_contents["Entities"][it->get_entity_data().entity_name]["rotation"]["z"]               = it->get_entity_data().rotation.z;

        json_contents["Entities"][it->get_entity_data().entity_name]["overall_color"]["r"]          = it->get_entity_data().overall_color.r;
        json_contents["Entities"][it->get_entity_data().entity_name]["overall_color"]["g"]          = it->get_entity_data().overall_color.g;
        json_contents["Entities"][it->get_entity_data().entity_name]["overall_color"]["b"]          = it->get_entity_data().overall_color.b;
        json_contents["Entities"][it->get_entity_data().entity_name]["overall_color"]["a"]          = it->get_entity_data().overall_color.a;


        // json_contents["Entities"][it->get_entity_data().entity_name]["materials"]["has_mlt"]       = it->get_entity_data().materials.has_mtl;
        json_contents["Entities"][it->get_entity_data().entity_name]["materials"]["has_texture"]    = it->get_entity_data().materials.has_texture;
        json_contents["Entities"][it->get_entity_data().entity_name]["materials"]["has_ambiant"]    = it->get_entity_data().materials.has_ambiant;
        json_contents["Entities"][it->get_entity_data().entity_name]["materials"]["has_diffuse"]    = it->get_entity_data().materials.has_diffuse;
        json_contents["Entities"][it->get_entity_data().entity_name]["materials"]["has_specular"]   = it->get_entity_data().materials.has_specular;
        json_contents["Entities"][it->get_entity_data().entity_name]["materials"]["has_normal"]     = it->get_entity_data().materials.has_normal;
        // json_contents["Entities"][it->get_entity_data().entity_name]["materials"]["has_alpha"]     = it->get_entity_data().materials.has_alpha;

        json_contents["Entities"][it->get_entity_data().entity_name]["materials"]["shininess"]      = it->get_entity_data().materials.shininess;
        json_contents["Entities"][it->get_entity_data().entity_name]["materials"]["ior"]            = it->get_entity_data().materials.ior;
        json_contents["Entities"][it->get_entity_data().entity_name]["materials"]["opacity"]        = it->get_entity_data().materials.opacity;

        
        json_contents["Entities"][it->get_entity_data().entity_name]["properties"]["physical_body"]["mass"]     = it->get_entity_data().properties.physical_body.mass;
        json_contents["Entities"][it->get_entity_data().entity_name]["properties"]["physical_body"]["charge"]   = it->get_entity_data().properties.physical_body.charge;
	}

	std::filebuf entities_json;
    entities_json.open(scene_path + "/entities.json", std::ios::binary | std::ios::out);
	entities_json.sputn(json_contents.dump(4).c_str(), json_contents.dump(4).size());
	entities_json.close();

    AG_CORE_INFO("[Save Scene/SceneManager::ApplicationSceneManager - save_scene] Successfully saved scene to path : \'" + scene_path + "\'!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::save_scene()
{
    return save_scene_to_path(m_ScenePath);
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::close_current_scene()
{
    save_scene();
    m_ScenePath     = AG_SCENE_PATH_NONE;
    m_SceneClosed   = true;
    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = scene manip = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = entity misc = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::SceneManager::ApplicationSceneManager::create_entity(
    const Agos::Entities::Entity& entity_data
)
{
    // ! ======================== try to make sure its got enough room for allocation
    // m_ScenesEntities.push_back(entity_properties);
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::show_entity(
    const size_t& id
)
{
    m_LookFor(id)->show();
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::hide_entity(
    const size_t& id
)
{
    m_LookFor(id)->hide();
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::set_entity_type(
    const size_t&                                       entity_id,
    const Entities::EntityData::Properties::EntityType& type
)
{
    m_LookFor(entity_id)->get_entity_data().properties.entity_type = type;
    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = entity misc = = = = = = = = = = = = = = = = = = = =


// * = = = = = = = = = = = = = = = = = = = = entity misc = = = = = = = = = = = = = = = = = = = =
std::shared_ptr<Agos::Entities::Entity>& Agos::SceneManager::ApplicationSceneManager::m_LookFor(
    const size_t& entity_id
)
{
    for (std::shared_ptr<Entities::Entity>& it : m_ScenesEntities)
    {
        if (it->get_entity_id() == entity_id)
        {
            return it;
        }
    }
    AG_CORE_WARN("[LookForEntity/SceneManager::ApplicationSceneManager - m_LookFor] Failed to find entity with id : " + std::to_string(entity_id) + std::string(" !"));

    return m_ScenesEntities[0];
}

const std::shared_ptr<Agos::Entities::Entity>& Agos::SceneManager::ApplicationSceneManager::m_LookFor(
    const size_t& entity_id
)   const
{
    for (const std::shared_ptr<Entities::Entity>& it : m_ScenesEntities)
    {
        if (it->get_entity_id() == entity_id)
        {
            return it;
        }
    }
    AG_CORE_WARN("[LookForEntity/SceneManager::ApplicationSceneManager - m_LookFor] Failed to find entity with id : " + std::to_string(entity_id) + std::string(" !"));

    return m_ScenesEntities[0];
}


Agos::AgResult Agos::SceneManager::ApplicationSceneManager::m_LoadEntityObjData(Agos::Entities::Entity& entity)
{
    bool has_mtl = false;
    glm::vec3 polygons_color({1.0f, 1.0f, 1.0f});

    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    reader_config.mtl_search_path = "";

    if (!reader.ParseFromFile(entity.get_entity_data().obj_file_path, reader_config))
    {
        if (!reader.Error().empty())
        {
            AG_CORE_ERROR("[LoadEntityObjData/SceneManager::ApplicationSceneManager - m_LoadEntityObjData] TinyObjReader : " + reader.Error());
        }
        if (!reader.Warning().empty())
        {
            AG_CORE_ERROR("[LoadEntityObjData/SceneManager::ApplicationSceneManager - m_LoadEntityObjData] TinyObjReader : " + reader.Warning());
        }
        return AG_FAILED_TO_READ_OBJ_FILE;
    }
    if (!reader.Warning().empty())
    {
        AG_CORE_WARN("[LoadEntityObjData/SceneManager::ApplicationSceneManager - m_LoadEntityObjData] TinyObjReader : " + reader.Warning());
    }

    attrib      = std::move( reader.GetAttrib()     );
    shapes      = std::move( reader.GetShapes()     );
    materials   = std::move( reader.GetMaterials()  );

    if (!materials.empty())
        has_mtl = true;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    bool normals_included = false;

    for (const tinyobj::shape_t& shape : shapes)
    {
        for (const tinyobj::index_t& index : shape.mesh.indices)
        {
            Vertex vertex{};

            // position
            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            // texture
            if (index.texcoord_index >= 0 && !has_mtl)
            {
                vertex.textCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
            }

            // color
            if (attrib.colors.size() != 0)
            {
                tinyobj::real_t red   = attrib.colors[3 * (index.vertex_index) + 0];
                tinyobj::real_t green = attrib.colors[3 * (index.vertex_index) + 1];
                tinyobj::real_t blue  = attrib.colors[3 * (index.vertex_index) + 2];
                vertex.color = std::move(glm::vec3{red, green, blue} * polygons_color);
            }
            else
            {
                vertex.color = polygons_color;
            }
            
            // normals
            if (index.normal_index >= 0)
            {
                normals_included = true;
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    if (!normals_included)
    {
        for (size_t vertex = 1; vertex < indices.size(); vertex += 3)
        {
            glm::vec3 p1_p2 {
                vertices[indices[vertex]].position.x - vertices[indices[vertex - 1]].position.x,
                vertices[indices[vertex]].position.y - vertices[indices[vertex - 1]].position.y,
                vertices[indices[vertex]].position.z - vertices[indices[vertex - 1]].position.z
            };
            glm::vec3 p1_p3 {
                vertices[indices[vertex + 1]].position.x - vertices[indices[vertex - 1]].position.x,
                vertices[indices[vertex + 1]].position.y - vertices[indices[vertex - 1]].position.y,
                vertices[indices[vertex + 1]].position.z - vertices[indices[vertex - 1]].position.z
            };
            glm::vec3 normal = glm::cross(p1_p2, p1_p3);
            vertices[indices[vertex - 1]].normal = normal;
            vertices[indices[vertex]].normal = normal;
            vertices[indices[vertex + 1]].normal = normal;
        }
    }
    entity.get_entity_data().vertices   = std::move(vertices);
    entity.get_entity_data().indices    = std::move(indices);

    for(Vertex& it : entity.get_entity_data().vertices)
    {
        it.position += entity.get_entity_data().translation;
    }

    return AG_SUCCESS;
}

// we basically fill in all the different paths to the corresponding textures since loading a texture to the GPU would either be the renderer's job
Agos::AgResult Agos::SceneManager::ApplicationSceneManager::m_LoadEntityTextures(Agos::Entities::Entity& entity)
{
    m_FindTexturePath   (entity);
    m_FindAmbiantPath   (entity);
    m_FindDiffusePath   (entity);
    m_FindSpecularPath  (entity);
    m_FindNormalPath    (entity);
    m_FindAlphaPath     (entity);
    return AG_SUCCESS;
}

// * = * = * = * = * = * = * = * = * = * = * = * maps miscs and utils * = * = * = * = * = * = * = * = * = * = * = *
std::string Agos::SceneManager::ApplicationSceneManager::m_FindMapPath(
    Entities::Entity&   entity,
    const std::string&  map_specifier
)
{
    for (const std::filesystem::directory_entry& file_entry : std::filesystem::directory_iterator(m_ScenePath + "/Entities/" + entity.get_entity_data().entity_name))
    {
        if ( std::string(file_entry.path().stem()) == std::string(entity.get_entity_data().entity_name + map_specifier) )
        {
            return std::move(file_entry.path().string());
        }
    }
    return std::move(AG_ENTITY_NO_MAP_FOUND);
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::m_FindTexturePath(
    Entities::Entity& entity
)
{
    entity.get_entity_data().materials.texture_path = m_FindMapPath(entity, "-texture");
    if ( entity.get_entity_data().materials.texture_path == std::string(AG_ENTITY_NO_MAP_FOUND) && entity.get_entity_data().materials.has_texture)
    {
        AG_CORE_WARN("[Load Entity's Maps - Find Ambiant Path/SceneManager::ApplicationSceneManager - m_FindAmbiantPath] Entity \'"
            + entity.get_entity_data().entity_name + "\' has a texture, but none were found at : \'"
            + m_ScenePath + "/Entities/" + entity.get_entity_data().entity_name + "\'!");
    }
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::m_FindAmbiantPath(
    Entities::Entity& entity
)
{
    entity.get_entity_data().materials.ambiant_path = m_FindMapPath(entity, "-ambiant");
    if ( entity.get_entity_data().materials.ambiant_path == std::string(AG_ENTITY_NO_MAP_FOUND) && entity.get_entity_data().materials.has_ambiant)
    {
        AG_CORE_WARN("[Load Entity's Maps - Find Ambiant Path/SceneManager::ApplicationSceneManager - m_FindAmbiantPath] Entity \'"
            + entity.get_entity_data().entity_name + "\' has an ambiant map, but none were found at : \'"
            + m_ScenePath + "/Entities/" + entity.get_entity_data().entity_name + "\'!");
    }
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::m_FindDiffusePath(
    Entities::Entity& entity
)
{
    entity.get_entity_data().materials.diffuse_path = m_FindMapPath(entity, "-diffuse");
    if ( entity.get_entity_data().materials.diffuse_path == std::string(AG_ENTITY_NO_MAP_FOUND) && entity.get_entity_data().materials.has_diffuse)
    {
        AG_CORE_WARN("[Load Entity's Maps - Find Ambiant Path/SceneManager::ApplicationSceneManager - m_FindAmbiantPath] Entity \'"
            + entity.get_entity_data().entity_name + "\' has a diffuse map, but none were found at : \'"
            + m_ScenePath + "/Entities/" + entity.get_entity_data().entity_name + "\'!");
    }
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::m_FindSpecularPath(
    Entities::Entity& entity
)
{
    entity.get_entity_data().materials.specular_path = m_FindMapPath(entity, "-specular");
    if ( entity.get_entity_data().materials.specular_path == std::string(AG_ENTITY_NO_MAP_FOUND) && entity.get_entity_data().materials.has_specular)
    {
        AG_CORE_WARN("[Load Entity's Maps - Find Ambiant Path/SceneManager::ApplicationSceneManager - m_FindAmbiantPath] Entity \'"
            + entity.get_entity_data().entity_name + "\' has a specular map, but none were found at : \'"
            + m_ScenePath + "/Entities/" + entity.get_entity_data().entity_name + "\'!");
    }
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::m_FindNormalPath(
    Entities::Entity& entity
)
{
    entity.get_entity_data().materials.normal_path = m_FindMapPath(entity, "-normal");
    if ( entity.get_entity_data().materials.normal_path == std::string(AG_ENTITY_NO_MAP_FOUND) && entity.get_entity_data().materials.has_normal)
    {
        AG_CORE_WARN("[Load Entity's Maps - Find Ambiant Path/SceneManager::ApplicationSceneManager - m_FindAmbiantPath] Entity \'"
            + entity.get_entity_data().entity_name + "\' has a normal map, but none were found at : \'"
            + m_ScenePath + "/Entities/" + entity.get_entity_data().entity_name + "\'!");
    }
    return AG_SUCCESS;
}

Agos::AgResult Agos::SceneManager::ApplicationSceneManager::m_FindAlphaPath(
    Entities::Entity& entity
)
{
    entity.get_entity_data().materials.alpha_path = m_FindMapPath(entity, "-alpha");
    if ( entity.get_entity_data().materials.alpha_path == std::string(AG_ENTITY_NO_MAP_FOUND) && entity.get_entity_data().materials.has_alpha)
    {
        AG_CORE_WARN("[Load Entity's Maps - Find Ambiant Path/SceneManager::ApplicationSceneManager - m_FindAmbiantPath] Entity \'"
            + entity.get_entity_data().entity_name + "\' has an alpha map, but none were found at : \'"
            + m_ScenePath + "/Entities/" + entity.get_entity_data().entity_name + "\'!");
    }
    return AG_SUCCESS;
}
// * = * = * = * = * = * = * = * = * = * = * = * maps miscs and utils * = * = * = * = * = * = * = * = * = * = * = *


inline size_t Agos::SceneManager::ApplicationSceneManager::m_GenerateNewEntityId()
{
    // !*!*! ENTITY ID GENERATION ALGORITHM REQUIRED
    return 0;
}
// * = = = = = = = = = = = = = = = = = = = = entity misc = = = = = = = = = = = = = = = = = = = =
// ** AgosGE ApplicationSceneManager class ==========================================================================
