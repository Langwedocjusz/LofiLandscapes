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

class FPCamera {
public:
    FPCamera();
    ~FPCamera();

    void Update(float deltatime);

    glm::mat4 getViewMatrix() {return m_Camera.getViewMatrix();}
    glm::mat4 getProjMatrix(unsigned int width, unsigned int height);

    void OnWindowResize(unsigned int width, unsigned int height);
    void OnKeyPressed(int keycode, bool repeat);
    void OnKeyReleased(int keycode);
    void OnMouseMoved(float x, float y, unsigned int width, unsigned int height);
private:
    bool m_MouseInit = true;
    float m_MouseLastX = 0.0f, m_MouseLastY = 0.0f;

    Camera m_Camera;
};
