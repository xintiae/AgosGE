#include "Agos/src/renderer/camera/camera.h"


// ** CameraObject ==================================================================================================================================
// * = = = = = = = = = = = = = = = = = = = = constructors, destructors, move scemantics = = = = = = = = = = = = = = = = = = = =
Agos::Clipping::CameraObject::CameraObject()
{
    m_Position      = glm::vec3(10.0f, 10.0f, 10.0f);
    m_Direction     = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - m_Position);
    m_Upwards       = glm::vec3(0.0f, 1.0f, 0.0f);
    m_Speed         = 1.0f;
    m_Zfar          = 500.0f;
    m_Znear         = 0.0625f;

    compute_initial_rotation();
}

Agos::Clipping::CameraObject::CameraObject(
    const glm::vec3&    position,
    const glm::vec3&    look_at,
    const glm::vec3&    up,
    const float&        speed,
    const float&        z_far,
    const float&        z_near
)   : m_Speed   (speed),
    m_Zfar      (z_far),
    m_Znear     (z_near)
{
    m_Position      = position;
    m_Direction     = glm::normalize(look_at - position);
    m_Upwards       = glm::normalize(up);
    compute_initial_rotation();
}

Agos::Clipping::CameraObject::~CameraObject()
{
}

Agos::Clipping::CameraObject::CameraObject(const Agos::Clipping::CameraObject& other)
{
    this->m_Position    = other.m_Position;
    this->m_Direction   = other.m_Direction;
    this->m_Upwards     = other.m_Upwards;
    this->m_Pitch       = other.m_Pitch;
    this->m_Yaw         = other.m_Yaw;
    this->m_Speed       = other.m_Speed;
}

Agos::Clipping::CameraObject::CameraObject(Agos::Clipping::CameraObject&& other)
{
    this->m_Position    = std::move(other.m_Position);
    this->m_Direction   = std::move(other.m_Direction);
    this->m_Upwards     = std::move(other.m_Upwards);
    this->m_Pitch       = std::move(other.m_Pitch);
    this->m_Yaw         = std::move(other.m_Yaw);
    this->m_Speed       = std::move(other.m_Speed);
}

Agos::Clipping::CameraObject& Agos::Clipping::CameraObject::operator=(const Agos::Clipping::CameraObject& other)
{
    this->m_Position    = other.m_Position;
    this->m_Direction   = other.m_Direction;
    this->m_Upwards     = other.m_Upwards;
    this->m_Pitch       = other.m_Pitch;
    this->m_Yaw         = other.m_Yaw;
    this->m_Speed       = other.m_Speed;
    return *this;
}

Agos::Clipping::CameraObject& Agos::Clipping::CameraObject::operator=(Agos::Clipping::CameraObject&& other)
{
    this->m_Position    = std::move(other.m_Position);
    this->m_Direction   = std::move(other.m_Direction);
    this->m_Upwards     = std::move(other.m_Upwards);
    this->m_Pitch       = std::move(other.m_Pitch);
    this->m_Yaw         = std::move(other.m_Yaw);
    this->m_Speed       = std::move(other.m_Speed);
    return *this;
}
// * = = = = = = = = = = = = = = = = = = = = constructors, destructors, move scemantics = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = misc = = = = = = = = = = = = = = = = = = = =
glm::vec3 Agos::Clipping::CameraObject::right()
{
    return glm::normalize(glm::cross(m_Direction, m_Upwards));
}

void Agos::Clipping::CameraObject::shrink_orientation()
{
    if (m_Pitch > 90.0f)
    {
        m_Pitch = 89.5f;
    }
    else if (m_Pitch < -90.0f)
    {
        m_Pitch = -89.5f;
    }


}

void Agos::Clipping::CameraObject::compute_initial_rotation()
{
    m_Pitch = glm::degrees(glm::asin(m_Direction.y));
    m_Yaw   = glm::degrees(glm::acos(m_Direction.x / glm::cos(glm::radians(m_Pitch)))) + 90.0f;
}
// * = = = = = = = = = = = = = = = = = = = = base computation and misc = = = = = = = = = = = = = = = = = = = =
// ** CameraObject ==================================================================================================================================


/*
void Agos::AgCameraObject::compute_camera_basis()
{
    m_CameraRight     = glm::normalize(glm::cross(this->m_Up, this->m_CameraOppositeDirection));
    m_CameraUp        = glm::normalize(glm::cross(this->m_CameraOppositeDirection, this->m_CameraRight));
}

void Agos::AgCameraObject::calculate_adequate_camera_speed(const std::chrono::high_resolution_clock::time_point& start_time)
{
    std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    m_CameraSpeed = deltaTime * AG_CAMERA_SPEED_DEFAULT_PER_UNIT_SPEED * AG_CAMERA_SPEED_SCALING_FACTOR;
}
*/
