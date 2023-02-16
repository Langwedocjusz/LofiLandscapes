#include "Camera.h"

#include "Keycodes.h"

Camera::Camera(glm::vec3 pos, glm::vec3 wup, float pitch, float yaw)
    : m_Pos(pos), m_WorldUp(wup), m_Pitch(pitch), m_Yaw(yaw)
{
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(m_Pos, m_Pos + m_Front, m_Up);
}

void Camera::ProcessKeyboard(float deltatime) {
    float velocity = deltatime * m_Settings.Speed;

    if ((m_Movement & CameraMovement::Forward) != CameraMovement::None)
        m_Pos += velocity * m_Front;
    if ((m_Movement & CameraMovement::Backward) != CameraMovement::None)
        m_Pos -= velocity * m_Front;
    if ((m_Movement & CameraMovement::Left) != CameraMovement::None)
        m_Pos -= velocity * m_Right;
    if ((m_Movement & CameraMovement::Right) != CameraMovement::None)
        m_Pos += velocity * m_Right;

}

void Camera::ProcessMouse(float xoffset, float yoffset) {
    m_Pitch += m_Settings.Sensitivity * yoffset;
    m_Yaw += m_Settings.Sensitivity * xoffset;

    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;

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

FPCamera::FPCamera() {}
FPCamera::~FPCamera() {}

void FPCamera::Update(float deltatime) {
    ProcessKeyboard(deltatime);
}

glm::mat4 FPCamera::getProjMatrix(float aspect) {
    return glm::perspective(
        glm::radians(m_Settings.Fov), 
        aspect, m_NearPlane, m_FarPlane
    );
}

void FPCamera::OnKeyPressed(int keycode, bool repeat) {
    if (!repeat) {
        switch(keycode) {
            case LOFI_KEY_W: {
                m_Movement = m_Movement | CameraMovement::Forward;
                break;
            }
            case LOFI_KEY_S: {
                m_Movement = m_Movement | CameraMovement::Backward;
                break;
            }
            case LOFI_KEY_A: {
                m_Movement = m_Movement | CameraMovement::Left;
                break;
            }
            case LOFI_KEY_D: {
                m_Movement = m_Movement | CameraMovement::Right;
                break;
            }
        }
    }

}

void FPCamera::OnKeyReleased(int keycode) {
    switch(keycode) {
        case LOFI_KEY_W: {
            m_Movement = m_Movement & ~CameraMovement::Forward;
            break;
        }
        case LOFI_KEY_S: {
            m_Movement = m_Movement & ~CameraMovement::Backward;
            break;
        }
        case LOFI_KEY_A: {
            m_Movement = m_Movement & ~CameraMovement::Left;
            break;
        }
        case LOFI_KEY_D: {
            m_Movement = m_Movement & ~CameraMovement::Right;
            break;
        }
    }
}

void FPCamera::OnMouseMoved(float x, float y, unsigned int width, unsigned int height) {
    float xpos = x/width;
    float ypos = y/height;

    if (m_MouseInit) {
        m_MouseLastX = xpos;
        m_MouseLastY = ypos;
        m_MouseInit = false;
    }

    float xoffset = xpos - m_MouseLastX;
    float yoffset = m_MouseLastY - ypos;

    m_MouseLastX = xpos;
    m_MouseLastY = ypos;

    const float max_offset = 0.1f;

    if (abs(xoffset) > max_offset)
        xoffset = (xoffset > 0.0f) ? max_offset : -max_offset;
    
    if (abs(yoffset) > max_offset)
        yoffset = (yoffset > 0.0f) ? max_offset : -max_offset;

    ProcessMouse(xoffset, yoffset);
}

//Operator overloads:

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

bool operator==(const CameraSettings& lhs, const CameraSettings& rhs) {
    return (lhs.Speed == rhs.Speed) && (lhs.Sensitivity == rhs.Sensitivity)
        && (lhs.Fov == rhs.Fov);
}

bool operator!=(const CameraSettings& lhs, const CameraSettings& rhs) {
    return !(lhs == rhs);
}