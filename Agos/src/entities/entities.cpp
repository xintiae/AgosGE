#include "Agos/src/entities/entities.h"


// ** AgosGE Entity class ==========================================================================
// * = = = = = = = = = = = = = = = = = = = = constructor, destructor = = = = = = = = = = = = = = = = = = = =
Agos::Entities::Entity::Entity()
    : m_Id              (0),
    m_Data              (),
    m_ShouldBeShown     (true),
    m_GPULoaded         (false),
    m_EntityDestroyed   (false)
{
}

Agos::Entities::Entity::Entity(
    const size_t& id
)
    : m_Id              (id),
    m_Data              (),
    m_ShouldBeShown     (true),
    m_EntityDestroyed   (false)
{
}

Agos::Entities::Entity::Entity(
    const size_t& id,
    const Agos::Entities::EntityData::Data& data
)
    : m_Id              (id),
    m_Data              (),
    m_ShouldBeShown     (true),
    m_EntityDestroyed   (false)

{
    this->m_Data = std::move(data);
}

Agos::Entities::Entity::~Entity()
{
    destroy_entity();    
}

// * = * = * = * = * move scemantics | constructors and operators * = * = * = * = *
Agos::Entities::Entity::Entity(const Agos::Entities::Entity& other)
{
    m_Id                = other.m_Id;
    m_Data              = other.m_Data;
    m_ShouldBeShown     = other.m_ShouldBeShown;
    m_EntityDestroyed   = other.m_EntityDestroyed;
}

Agos::Entities::Entity::Entity(Agos::Entities::Entity&& other)
{
    m_Id                = std::move(other.m_Id);
    m_Data              = std::move(other.m_Data);
    m_ShouldBeShown     = std::move(other.m_ShouldBeShown);
    m_EntityDestroyed   = std::move(other.m_EntityDestroyed);
}

Agos::Entities::Entity& Agos::Entities::Entity::operator=(const Agos::Entities::Entity& other)
{
    m_Id                = other.m_Id;
    m_Data              = other.m_Data;
    m_ShouldBeShown     = other.m_ShouldBeShown;
    m_EntityDestroyed   = other.m_EntityDestroyed;
    return *this;
}

Agos::Entities::Entity& Agos::Entities::Entity::operator=(Agos::Entities::Entity&& other)
{
    m_Id                = std::move(other.m_Id);
    m_Data              = std::move(other.m_Data);
    m_ShouldBeShown     = std::move(other.m_ShouldBeShown);
    m_EntityDestroyed   = std::move(other.m_EntityDestroyed);
    return *this;
}

// * = = = = = = = = = = = = = = = = = = = = constructor, destructor = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = entity manipulation = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::Entities::Entity::show()
{
    m_ShouldBeShown = true;
    return AG_SUCCESS;
}

Agos::AgResult Agos::Entities::Entity::hide()
{
    m_ShouldBeShown = false;
    return AG_SUCCESS;
}

Agos::AgResult Agos::Entities::Entity::destroy_entity()
{
    if (!m_EntityDestroyed)
    {
        m_Id = 0;
        m_EntityDestroyed = true;
        m_Data.entity_name = std::move(AG_ENTITY_NAME_DESTROYED_ENTITY);

        m_Data.obj_file_path = std::move(AG_ENTITY_OBJ_FILE_PATH_DESTROYED);
        m_Data.vertices.clear();
        m_Data.indices.clear();

        return AG_SUCCESS;
    }
    return AG_ENTITY_ALREADY_DESTROYED;
}
// * = = = = = = = = = = = = = = = = = = = = entity manipulation = = = = = = = = = = = = = = = = = = = =


// ** AgosGE Entity class ==========================================================================
