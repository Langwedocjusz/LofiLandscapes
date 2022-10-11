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
    unsigned int m_N = 192;
    float m_L = 10.0f;

    std::vector<float> m_VertexData;
    std::vector<unsigned int> m_IndexData;
    unsigned int m_VAO=0, m_VBO=0, m_EBO=0;

    float m_QuadVertexData[12] = {-1.0f, 1.0f, 1.0f,
                                   1.0f, 1.0f, 1.0f,
                                   1.0f,-1.0f, 1.0f,
                                  -1.0f,-1.0f, 1.0f };
    unsigned int m_QuadIndexData[6] = {0,1,3, 1,2,3};
    unsigned int m_QuadVAO, m_QuadVBO, m_QuadEBO;
    unsigned int m_FBO, m_TargetTexture;

    float m_ClearColor[3] = {0.2f, 0.2f, 0.2f};

    unsigned int m_WindowWidth, m_WindowHeight;
    Shader m_Shader;
    Shader m_QuadShader;
    FPCamera m_Camera;

    glm::mat4 m_MVP = glm::mat4(1.0f);
};
