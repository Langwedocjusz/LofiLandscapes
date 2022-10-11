#pragma once

#include "Shader.h"
#include "Camera.h"
#include "Scene.h"

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
    unsigned int m_N = 192;
    float m_L = 10.0f;

    unsigned int m_FBO, m_TargetTexture;

    float m_ClearColor[3] = {0.2f, 0.2f, 0.2f};

    unsigned int m_WindowWidth, m_WindowHeight;

    Shader m_Shader;
    Shader m_QuadShader;
    FPCamera m_Camera;

    glm::mat4 m_MVP = glm::mat4(1.0f);

    Scene m_Scene;
};
