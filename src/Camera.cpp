#include "Camera.h"

#include "Keycodes.h"

#include "imgui.h"
#include "ImGuiUtils.h"

bool Plane::IsInFront(const AABB& aabb, float scale_y) const {
    const glm::vec3 scale = {1.0f, scale_y, 1.0f};
    const glm::vec3 extents = scale * aabb.Extents;
    const glm::vec3 normal = glm::abs(Normal);

    //Absolute value of the projection of the extents 
    //vector onto subspace orthogonal to the plane
    const float r = glm::dot(extents, normal);

    //Signed distance of center to plane
    float sd = -glm::dot(Origin - scale*aabb.Center, Normal);

    //This statement is tautologically true in front of the plane
    //which is good since we should return true in that case
    return -r <= sd;
}

bool Frustum::IsInFrustum(const AABB& aabb, float scale_y) const {
    return Top.IsInFront(aabb, scale_y)
        && Bottom.IsInFront(aabb, scale_y)
        && Left.IsInFront(aabb, scale_y)
        && Right.IsInFront(aabb, scale_y)
        && Near.IsInFront(aabb, scale_y)
        && Far.IsInFront(aabb, scale_y);
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

    if ((m_Movement & Forward) != None)
        m_Pos += velocity * m_Front;

    if ((m_Movement & Backward) != None)
        m_Pos -= velocity * m_Front;

    if ((m_Movement & Left) != None)
        m_Pos -= velocity * m_Right;

    if ((m_Movement & Right) != None)
        m_Pos += velocity * m_Right;
}

void Camera::ProcessMouse(float xoffset, float yoffset, float aspect) {
    m_Pitch += m_Sensitivity * yoffset;
    m_Yaw   += m_Sensitivity * xoffset;

    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;

    updateVectors();
    updateFrustum(aspect);
}

void Camera::updateVectors() {
   m_Front.x = glm::cos(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
   m_Front.y = glm::sin(glm::radians(m_Pitch));
   m_Front.z = glm::sin(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
   m_Front = glm::normalize(m_Front);

   m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
   m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

void Camera::updateFrustum(float aspect) {
    //Camera position in coordinate system tied to the grid:
    const glm::vec3 pos = glm::vec3(0.0f, m_Pos.y, 0.0f);

    const float halfV = m_FarPlane * tanf(glm::radians(m_Fov));
    const float halfH = aspect * halfV; //aspect = horizontal/vertical

    const glm::vec3 farFront = m_FarPlane * m_Front;

    m_Frustum.Near = { pos + m_NearPlane * m_Front,  m_Front };
    m_Frustum.Far  = { pos + m_FarPlane  * m_Front, -m_Front };

    //Normals are usually normalized, but we're only going to use them
    //to compare two values, each linear in those vectors
    //so it doesn't matter
    m_Frustum.Left  = { pos, (glm::cross(farFront - halfH * m_Right, m_Up)) };
    m_Frustum.Right = { pos, (glm::cross(m_Up, farFront + halfH * m_Right)) };

    m_Frustum.Top    = { pos, (glm::cross(farFront + halfV * m_Up, m_Right)) };
    m_Frustum.Bottom = { pos, (glm::cross(m_Right, farFront - halfV * m_Up)) };
}

bool Camera::IsInFrustum(const AABB& aabb, float scale_y) const {
    return m_Frustum.IsInFrustum(aabb, scale_y);
}

void Camera::OnImGui(bool& open) {
    ImGui::Begin("Camera", &open, ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::Columns(2, "###col");
    ImGuiUtils::SliderFloat("Speed", &(m_Speed), 0.0, 10.0f);
    ImGuiUtils::SliderFloat("Sensitivity", &(m_Sensitivity), 0.0f, 200.0f);
    ImGuiUtils::SliderFloat("Fov", &(m_Fov), 0.0f, 90.0f);
    ImGui::Columns(1, "###col");
    ImGui::End();
}

FPCamera::FPCamera() {}
FPCamera::~FPCamera() {}

void FPCamera::Update(float deltatime) {
    ProcessKeyboard(deltatime);
}

glm::mat4 FPCamera::getProjMatrix(float aspect) {
    return glm::perspective(
        glm::radians(m_Fov), aspect, m_NearPlane, m_FarPlane
    );
}

void FPCamera::OnKeyPressed(int keycode, bool repeat) {
    if (!repeat) {
        switch(keycode) {
            case LOFI_KEY_W: {
                m_Movement = m_Movement | Forward;
                break;
            }
            case LOFI_KEY_S: {
                m_Movement = m_Movement | Backward;
                break;
            }
            case LOFI_KEY_A: {
                m_Movement = m_Movement | Left;
                break;
            }
            case LOFI_KEY_D: {
                m_Movement = m_Movement | Right;
                break;
            }
        }
    }

}

void FPCamera::OnKeyReleased(int keycode) {
    switch(keycode) {
        case LOFI_KEY_W: {
            m_Movement = m_Movement & ~Forward;
            break;
        }
        case LOFI_KEY_S: {
            m_Movement = m_Movement & ~Backward;
            break;
        }
        case LOFI_KEY_A: {
            m_Movement = m_Movement & ~Left;
            break;
        }
        case LOFI_KEY_D: {
            m_Movement = m_Movement & ~Right;
            break;
        }
    }
}

void FPCamera::OnMouseMoved(float x, float y, unsigned int width, unsigned int height, float aspect) {
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

    ProcessMouse(xoffset, yoffset, aspect);
}