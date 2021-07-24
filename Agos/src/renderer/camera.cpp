#include "Agos/src/renderer/camera.h"

Agos::AgCameraObject::AgCameraObject(
    const glm::vec3& camera_pos,
    const glm::vec3& camera_direction,
    const float& camera_speed,
    const glm::vec3& up,
    const float& init_yaw,
    const float& init_pitch)
    : m_CameraPosition              (camera_pos),
    m_CameraOppositeDirection       (glm::normalize(camera_direction)),
    m_Up                            (up),
    m_CameraRight                   ( glm::normalize(glm::cross(m_Up, m_CameraOppositeDirection)) ),
    m_CameraUp                      ( glm::normalize(glm::cross(m_CameraOppositeDirection, m_CameraRight)) ),
    m_CameraSpeed                   (camera_speed),
    m_CameraYaw                     (init_yaw),
    m_CameraPitch                   (init_pitch)
{
}


Agos::AgCameraObject::~AgCameraObject()
{
}

void Agos::AgCameraObject::compute_camera_basis()
{
    m_CameraRight     = glm::normalize(glm::cross(this->m_Up, this->m_CameraOppositeDirection));
    m_CameraUp        = glm::normalize(glm::cross(this->m_CameraOppositeDirection, this->m_CameraRight));
}

void Agos::AgCameraObject::calculate_adequate_camera_speed(const std::chrono::_V2::high_resolution_clock::time_point& start_time)
{
    std::chrono::_V2::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    m_CameraSpeed = deltaTime * AG_CAMERA_DEFAULT_PER_UNIT_SPEED * 100;
}
