#pragma once

#include "Shader.h"
#include "Camera.h"

class Renderer {
public:
    Renderer(unsigned int width, unsigned int height);
    ~Renderer();

    void OnUpdate(float deltatime);
    void OnRender();
    void OnImGuiRender();

    void OnWindowResize(unsigned int width, unsigned int height);
    void OnKeyPressed(int keycode, bool repeat);
    void OnKeyReleased(int keycode);
    void OnMouseMoved(float x, float y);
private:
    bool m_ShowMenu = false;
    float m_ClearColor[3] = {0.2f, 0.2f, 0.2f};

    float m_QuadData[24] = {-0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
                            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
                             0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
                             0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f};

    unsigned int m_QuadIndices[6] = {0,1,2, 2,3,0};

    unsigned int m_VAO=0, m_VBO=0, m_EBO=0;
    unsigned int m_WindowWidth, m_WindowHeight;
    bool m_MouseInit = true;
    float m_MouseLastX=0.0f, m_MouseLastY=0.0f;

    Shader m_Shader;
    Camera m_Camera;

    glm::mat4 m_MVP = glm::mat4(1.0f);
};
