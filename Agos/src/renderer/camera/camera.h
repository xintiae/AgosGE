#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include AG_GLM_INCLUDE
// #include <chrono>

// #define AG_CAMERA_SPEED_DEFAULT_ABSOLUTE_SPEED (float)(1.0f)
// #define AG_CAMERA_SPEED_DEFAULT_PER_UNIT_SPEED (float)(1.0f)
// #define AG_CAMERA_SPEED_SCALING_FACTOR  100

namespace Agos
{
namespace Clipping
{


class CameraObject
{
private:
    glm::vec3   m_Position;
    glm::vec3   m_Direction;
    glm::vec3   m_Upwards;

    float       m_Pitch;
    float       m_Yaw;
    float       m_Speed;    // = 1.0f;
    float       m_Zfar;     // = 500.0f;
    float       m_Znear;    // = 0.0625f;

public:
    CameraObject();
    CameraObject(
        const glm::vec3&    position,
        const glm::vec3&    look_at     = glm::vec3(1.0f, 0.0f, 0.0f),
        const glm::vec3&    up          = glm::vec3(0.0f, 1.0f, 0.0f),
        const float&        speed       = 1.0f,
        const float&        z_far       = 500.0f,
        const float&        z_near      = 0.0625f
    );

    ~CameraObject();

    CameraObject(const CameraObject& other);
    CameraObject(CameraObject&& other);

    CameraObject& operator=(const CameraObject& other);
    CameraObject& operator=(CameraObject&& other);

    inline glm::vec3&   position    ()      { return m_Position;    }
    inline glm::vec3&   direction   ()      { return m_Direction;   }
    inline glm::vec3&   upwards     ()      { return m_Upwards;     }
    glm::vec3           right       ();

    // in degrees
    inline float&       pitch       ()      { return m_Pitch;       }
    // in degrees
    inline float&       yaw         ()      { return m_Yaw;         }
    inline float&       speed       ()      { return m_Speed;       }
    inline float&       z_far       ()      { return m_Zfar;        }
    inline float&       z_near      ()      { return m_Znear;       }

    // makes sure that m_Pitch always belongs to ]-pi/2; pi/2] radians
    void                shrink_orientation();

private:
    void compute_initial_rotation();
};  // ** class CameraObject


}   // ** namespace Camera (within namespace Agos)
}   // * namespace Agos
