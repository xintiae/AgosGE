#include "Agos/src/renderer/camera/camera.h"

Agos::AgCameraObject::AgCameraObject(
    const glm::vec3& camera_pos,
    const glm::vec3& camera_opposite_direction,
    const float& camera_speed,
    const glm::vec3& up)
    : m_CameraPosition              (camera_pos),
    m_CameraOppositeDirection       (glm::normalize(camera_opposite_direction)),
    m_Up                            (up),
    m_CameraRight                   ( glm::normalize(glm::cross(m_Up, m_CameraOppositeDirection)) ),
    m_CameraUp                      ( glm::normalize(glm::cross(m_CameraOppositeDirection, m_CameraRight)) ),
    m_CameraSpeed                   (camera_speed)
{
    glm::vec3 camera_direction = glm::vec3(-m_CameraOppositeDirection);
    m_CameraPitch = glm::degrees(glm::asin(camera_direction.y));
    m_CameraYaw = -(glm::degrees(glm::acos(camera_direction.x / glm::cos(m_CameraPitch))) - 90.0f);
}


Agos::AgCameraObject::~AgCameraObject()
{
}

void Agos::AgCameraObject::compute_camera_basis()
{
    m_CameraRight     = glm::normalize(glm::cross(this->m_Up, this->m_CameraOppositeDirection));
    m_CameraUp        = glm::normalize(glm::cross(this->m_CameraOppositeDirection, this->m_CameraRight));
}

void Agos::AgCameraObject::calculate_adequate_camera_speed(const std::chrono::high_resolution_clock::time_point& start_time)
{
    std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    m_CameraSpeed = deltaTime * AG_CAMERA_DEFAULT_PER_UNIT_SPEED * 100;
}
