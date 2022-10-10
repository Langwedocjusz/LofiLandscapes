#pragma once

#include "Shader.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void OnUpdate();
    void OnImGuiUpdate();

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
    Shader m_Shader;
};
