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
    glm::vec3 m_CameraOppositeDirection;
    glm::vec3 m_Up        = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 m_CameraRight;
    glm::vec3 m_CameraUp;

    float m_CameraSpeed;
    // in degrees
    float m_CameraYaw;
    // in degrees
    float m_CameraPitch;
    // in degrees
    // float m_CameraRoll;


    AgCameraObject(
        const glm::vec3& camera_pos,
        const glm::vec3& camera_direction,
        const float& camera_speed = AG_CAMERA_DEFAULT_ABSOLUTE_SPEED,
        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
        const float& init_yaw = 0.0f,
        const float& init_pitch = 0.0f);
    ~AgCameraObject();

    void compute_camera_basis();
    void calculate_adequate_camera_speed(const std::chrono::_V2::high_resolution_clock::time_point& start_time);
};  // class AgCameraObject

}   // namespace Agos
