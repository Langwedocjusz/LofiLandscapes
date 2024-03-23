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

//Meant to represent most extremal view directions that still fit
//inside the camera frustum.
struct FrustumExtents {
    glm::vec3 BottomLeft;
    glm::vec3 BottomRight;
    glm::vec3 TopLeft;
    glm::vec3 TopRight;
};

//Actual camera classes:

class Camera{
public:
    Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 wup = glm::vec3(0.0f, 1.0f, 0.0f),
           float pitch = 0.0f, float yaw = -90.0f);

    virtual glm::mat4 getViewMatrix() const;
    virtual glm::mat4 getProjMatrix() const = 0;
    virtual glm::mat4 getViewProjMatrix() const = 0;

    glm::vec3 getPos() const {return m_Pos;}
    glm::vec3 getPrevPos() const { return m_PrevPos; }
    glm::vec3 getFront() const {return m_Front;}
    glm::vec3 getRight() const { return m_Right; }
    glm::vec3 getUp() const { return m_Up; }

    float getNearPlane() const { return m_NearPlane; }
    float getFarPlane() const { return m_FarPlane; }

    //This is the same for all cameras as only construction of
    //frustum planes is assumed to differ (this is definitely the case for ortho and perspective)
    bool IsInFrustum(const AABB& aabb, float scale_y) const;

    virtual void OnImGui(bool& open) = 0;

protected:
    glm::vec3 m_Pos, m_PrevPos;
    glm::vec3 m_Front, m_Up, m_Right, m_WorldUp;
    float m_Pitch, m_Yaw;

   float m_NearPlane = 0.1f, m_FarPlane = 1000.0f;
    
    void updateVectors();

    Frustum m_Frustum;
    virtual void updateFrustum(float aspect) = 0; //Frustum construction depends on projection type
};

//Camera class implementing perspective projection
class PerspectiveCamera : public Camera {
public:
    glm::mat4 getProjMatrix() const override;
    glm::mat4 getViewProjMatrix() const override;

    float getFov() const { return m_Fov; }
    //Aspect is assumed to be x/y
    float getAspect() const { return m_Aspect; }
    //Inverse aspect is assumed to be y/x
    float getInvAspect() const { return m_InvAspect; }

    FrustumExtents getFrustumExtents() const;

    //Aspect is assumed to be x/y
    void setAspect(float aspect);

    void OnImGui(bool& open) override;
protected:
    //Aspect is assumed to be x/y
    virtual void updateFrustum(float aspect) override;

    float m_Fov = 45.0f;

    float m_Aspect = 1.0f, m_InvAspect = 1.0f;
};

//In principle we could also have orthographic or some more exotic projections here...

//First person camera - a perspective camera with keyboard/mouse movement implemented
class FPCamera : public PerspectiveCamera {
public:
    FPCamera();
    ~FPCamera();

    //Aspect is assumed to be x/y
    void Update(float aspect, float deltatime);
    
    glm::mat4 getViewMatrix() const override { return m_View; }
    glm::mat4 getProjMatrix() const override { return m_Proj; }
    glm::mat4 getViewProjMatrix() const override { return m_ViewProj; }

    float getSpeed() const { return m_Speed; }
    float getSensitivity() const { return m_Sensitivity; }

    void setMouseInit(bool p) { m_MouseInit = p; }

    void OnKeyPressed(int keycode, bool repeat);
    void OnKeyReleased(int keycode);
    void OnMouseMoved(float x, float y, uint32_t width, uint32_t height, float aspect);

    void OnImGui(bool& open) override;
private:
    void ProcessKeyboard(float deltatime);
    void ProcessMouse(float xoffset, float yoffset, float aspect);

    enum CameraMovement {
        None = 0,
        Forward = (1 << 0),
        Backward = (1 << 1),
        Left = (1 << 2),
        Right = (1 << 3)
    };

    int m_Movement = None;

    float m_Speed = 5.0f;
    float m_Sensitivity = 100.0f;

    bool m_MouseInit = true;
    float m_MouseLastX = 0.0f, m_MouseLastY = 0.0f;

    glm::mat4 m_Proj, m_View, m_ViewProj;
};
