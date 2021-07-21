#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include AG_GLM_INCLUDE
#include <chrono>


#define AG_CAMERA_DEFAULT_ABSOLUTE_SPEED (float)(1.0f)
#define AG_CAMERA_DEFAULT_PER_UNIT_SPEED (float)(1.0f)

namespace Agos
{

struct AG_API AgCameraObject
{
    glm::vec3 m_CameraPosition;
    glm::vec3 m_CameraLastPosition;
    glm::vec3 m_CameraTarget;
    // = glm::vec3(m_CameraPosition - m_CameraTarget)
    glm::vec3 m_CameraOppositeDirection;

    glm::vec3 m_Up        = glm::vec3(0.0f, 0.0f, 1.0f);
    // = glm::normalize(glm::cross(m_CameraUp, m_CameraOppositeDirection));
    glm::vec3 m_CameraRight;

    // = glm::cross(m_CameraOppositeDirection, m_CameraRight);
    glm::vec3 m_CameraUp;

    float m_CameraSpeed;


    AgCameraObject(
        const glm::vec3& camera_pos,
        const glm::vec3& camera_target,
        const float& camera_speed = AG_CAMERA_DEFAULT_ABSOLUTE_SPEED,   /** @b DO @b NOT USE determine_*/
        const glm::vec3& up = glm::vec3(0.0f, 0.0f, 1.0f));
    ~AgCameraObject();

    void compute_camera_direction();
    void compute_camera_directions();
    void compute_all();

    void calculate_adequate_camera_speed(const std::chrono::_V2::high_resolution_clock::time_point& start_time);
};  // class AgCameraObject

}   // namespace Agos
