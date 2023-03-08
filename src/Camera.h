#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

//CameraMovement operator~(CameraMovement x);
//CameraMovement operator|(CameraMovement x, CameraMovement y);
//CameraMovement operator&(CameraMovement x, CameraMovement y);

struct CameraSettings{
    float Speed = 5.0f;
    float Sensitivity = 100.0f;
    float Fov = 45.0f;
};

bool operator==(const CameraSettings& lhs, const CameraSettings& rhs);
bool operator!=(const CameraSettings& lhs, const CameraSettings& rhs);

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

    float getNearPlane() const { return m_NearPlane; }
    float getFarPlane() const { return m_FarPlane; }

    CameraSettings getSettings() const {return m_Settings;}
    void setSettings(CameraSettings x) {m_Settings = x;}

    void ProcessKeyboard(float deltatime);
    void ProcessMouse(float xoffset, float yoffset);
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

    CameraSettings m_Settings; 
    int m_Movement = None;

    //Won't be editable for now, so not included in settings
    float m_NearPlane = 0.1f, m_FarPlane = 1000.0f;

    void updateVectors();
};

class FPCamera : public Camera {
public:
    FPCamera();
    ~FPCamera();

    void Update(float deltatime);

    glm::mat4 getProjMatrix(float aspect);
    
    void OnWindowResize(unsigned int width, unsigned int height);
    void OnKeyPressed(int keycode, bool repeat);
    void OnKeyReleased(int keycode);
    void OnMouseMoved(float x, float y, unsigned int width, unsigned int height);

    void setMouseInit(bool p) {m_MouseInit = p;}
private:
    bool m_MouseInit = true;
    float m_MouseLastX = 0.0f, m_MouseLastY = 0.0f;
};
