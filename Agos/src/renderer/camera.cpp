#include "Agos/src/renderer/camera.h"

Agos::AgCameraObject::AgCameraObject(
    const glm::vec3& camera_pos,
    const glm::vec3& camera_target,
    const float& camera_speed,
    const glm::vec3& up)
    : m_CameraPosition              (camera_pos),
    m_CameraLastPosition            (camera_pos),
    m_CameraTarget                  (camera_target),
    m_CameraOppositeDirection       (m_CameraPosition - m_CameraTarget),
    m_Up                            (up),
    m_CameraRight                   (  glm::normalize(glm::cross(m_CameraUp, m_CameraOppositeDirection))  ),
    m_CameraUp                      (  glm::cross(m_CameraOppositeDirection, m_CameraRight)  ),
    m_CameraSpeed                   (camera_speed)
{
}


Agos::AgCameraObject::~AgCameraObject()
{
}

void Agos::AgCameraObject::compute_camera_direction()
{
    m_CameraOppositeDirection = std::move(glm::normalize(glm::vec3(m_CameraPosition - m_CameraTarget)));
    if (m_CameraOppositeDirection == glm::vec3(0.0f))
    {
        throw ;
    }
}

void Agos::AgCameraObject::compute_camera_directions()
{
    m_CameraRight     = glm::normalize(glm::cross(this->m_Up, this->m_CameraOppositeDirection));
    m_CameraUp        = glm::normalize(glm::cross(this->m_CameraOppositeDirection, this->m_CameraRight));
}

void Agos::AgCameraObject::compute_all()
{
    compute_camera_direction();
    compute_camera_directions();
}

void Agos::AgCameraObject::calculate_adequate_camera_speed(const std::chrono::_V2::high_resolution_clock::time_point& start_time)
{
    std::chrono::_V2::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    m_CameraSpeed = deltaTime * AG_CAMERA_DEFAULT_PER_UNIT_SPEED * 100;
}
