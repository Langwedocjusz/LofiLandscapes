#include "Camera.h"

CameraMovement operator~(CameraMovement x) {
    return static_cast<CameraMovement>(~static_cast<int>(x));
}

CameraMovement operator|(CameraMovement x, CameraMovement y) {
    return static_cast<CameraMovement>(static_cast<int>(x) 
                                     | static_cast<int>(y));
}

CameraMovement operator&(CameraMovement x, CameraMovement y) {
    return static_cast<CameraMovement>(static_cast<int>(x)
                                     & static_cast<int>(y));
}

Camera::Camera(glm::vec3 pos, glm::vec3 wup, float pitch, float yaw)
    : m_Pos(pos), m_WorldUp(wup), m_Pitch(pitch), m_Yaw(yaw)
{
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(m_Pos, m_Pos + m_Front, m_Up);
}

void Camera::ProcessKeyboard(float deltatime) {
    float velocity = deltatime * m_Speed;

    if ((Movement & CameraMovement::Forward) != CameraMovement::None)
        m_Pos += velocity * m_Front;
    if ((Movement & CameraMovement::Backward) != CameraMovement::None)
        m_Pos -= velocity * m_Front;
    if ((Movement & CameraMovement::Left) != CameraMovement::None)
        m_Pos -= velocity * m_Right;
    if ((Movement & CameraMovement::Right) != CameraMovement::None)
        m_Pos += velocity * m_Right;

}

void Camera::ProcessMouse(float xoffset, float yoffset) {
    m_Pitch += m_Sensitivity * yoffset;
    m_Yaw += m_Sensitivity * xoffset;

    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = 89.0f;

    updateVectors();
}

void Camera::updateVectors() {
   m_Front.x = glm::cos(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
   m_Front.y = glm::sin(glm::radians(m_Pitch));
   m_Front.z = glm::sin(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
   m_Front = glm::normalize(m_Front);

   m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
   m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
