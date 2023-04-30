#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

//Frustum culling setup based on
//https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling

struct AABB {
    glm::vec3 Center, Extents;
};

class Plane {
public:
    glm::vec3 Origin = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Normal = { 0.0f, 1.0f, 0.0f };

    bool IsInFront(const AABB& aabb, float scale_y) const;
};

class Frustum {
public:
    Frustum() = default;
    ~Frustum() = default;

    bool IsInFrustum(const AABB& aabb, float scale_y) const;

    Plane Top, Bottom, Left, Right, Near, Far;
};

class Camera{
public:
    Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 wup = glm::vec3(0.0f, 1.0f, 0.0f),
           float pitch = 0.0f, float yaw = -90.0f);

    glm::mat4 getViewMatrix();

    glm::vec3 getPos() const {return m_Pos;}
    glm::vec3 getFront() const {return m_Front;}
    glm::vec3 getRight() const { return m_Right; }
    glm::vec3 getUp() const { return m_Up; }

    float getFov() const { return m_Fov; }
    float getSpeed() const { return m_Speed; }
    float getSensitivity() const { return m_Sensitivity; }

    float getNearPlane() const { return m_NearPlane; }
    float getFarPlane() const { return m_FarPlane; }

    void ProcessKeyboard(float deltatime);
    void ProcessMouse(float xoffset, float yoffset, float aspect);

    bool IsInFrustum(const AABB& aabb, float scale_y) const;

    void OnImGui(bool& open);
protected:

    enum CameraMovement {
        None     =  0,
        Forward  = (1 << 0),
        Backward = (1 << 1),
        Left     = (1 << 2),
        Right    = (1 << 3)
    };

    glm::vec3 m_Pos;
    glm::vec3 m_Front, m_Up, m_Right, m_WorldUp;
    float m_Pitch, m_Yaw;

    int m_Movement = None;

    //Camera Settings
    float m_Speed = 5.0f;
    float m_Sensitivity = 100.0f;
    float m_Fov = 45.0f;
    float m_NearPlane = 0.1f, m_FarPlane = 1000.0f;
    
    void updateVectors();

    Frustum m_Frustum;
    void updateFrustum(float aspect);
};

class FPCamera : public Camera {
public:
    FPCamera();
    ~FPCamera();

    void Update(float deltatime);

    glm::mat4 getProjMatrix(float aspect);
    
    void OnKeyPressed(int keycode, bool repeat);
    void OnKeyReleased(int keycode);
    void OnMouseMoved(float x, float y, unsigned int width, unsigned int height, float aspect);

    void setMouseInit(bool p) {m_MouseInit = p;}
private:
    bool m_MouseInit = true;
    float m_MouseLastX = 0.0f, m_MouseLastY = 0.0f;
};
