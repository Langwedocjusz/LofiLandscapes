#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

enum class CameraMovement {
    None     =      0,
    Forward  = (1<<0),
    Backward = (1<<1),
    Left     = (1<<2),
    Right    = (1<<3)
};

CameraMovement operator~(CameraMovement x);
CameraMovement operator|(CameraMovement x, CameraMovement y);
CameraMovement operator&(CameraMovement x, CameraMovement y);

class Camera{
public:
    Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 wup = glm::vec3(0.0f, 1.0f, 0.0f),
           float pitch = 0.0f, float yaw = -90.0f);

    glm::mat4 getViewMatrix();
    float getFov() {return m_Fov;}

    void ProcessKeyboard(float deltatime);
    void ProcessMouse(float xoffset, float yoffset);
    
    CameraMovement Movement = CameraMovement::None;
private:
    glm::vec3 m_Pos;
    glm::vec3 m_Front, m_Up, m_Right, m_WorldUp;
    float m_Pitch, m_Yaw;

    float m_Speed = 1.0f;
    float m_Sensitivity = 100.0f;
    float m_Fov = 45.0f;

    void updateVectors();
};
