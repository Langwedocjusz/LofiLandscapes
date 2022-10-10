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
    void RestartMouse();
private:
    float m_ClearColor[3] = {0.2f, 0.2f, 0.2f};

    float m_QuadData[24] = {-0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
                            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
                             0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
                             0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f};

    unsigned int m_QuadIndices[6] = {0,1,2, 2,3,0};

    unsigned int m_VAO=0, m_VBO=0, m_EBO=0;
    unsigned int m_WindowWidth, m_WindowHeight;

    Shader m_Shader;
    FPCamera m_Camera;

    glm::mat4 m_MVP = glm::mat4(1.0f);
};
