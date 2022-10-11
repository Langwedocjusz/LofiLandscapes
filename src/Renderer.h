#pragma once

#include "Shader.h"
#include "Camera.h"
#include "Scene.h"

struct HeightmapParams{
    int Octaves = 4;
    int Resolution = 4096;
};

bool operator==(const HeightmapParams& lhs, const HeightmapParams& rhs);

class Renderer {
public:
    Renderer(unsigned int width, unsigned int height);
    ~Renderer();

    void OnUpdate(float deltatime);
    void OnRender();
    void OnImGuiRender();
    void RenderHeightmap();

    void OnWindowResize(unsigned int width, unsigned int height);
    void OnKeyPressed(int keycode, bool repeat);
    void OnKeyReleased(int keycode);
    void OnMouseMoved(float x, float y);
    void RestartMouse();
private:
    unsigned int m_N = 128;
    float m_L = 10.0f;

    unsigned int m_FBO, m_TargetTexture;

    float m_ClearColor[3] = {0.2f, 0.2f, 0.2f};

    unsigned int m_WindowWidth, m_WindowHeight;

    bool m_ShowTerrainMenu = false, m_ShowBackgroundMenu = false;
     
    bool m_Wireframe = false;
    HeightmapParams m_HeightmapParams;

    Shader m_ShadedShader, m_WireframeShader;
    Shader m_QuadShader;
    FPCamera m_Camera;

    glm::mat4 m_MVP = glm::mat4(1.0f);

    Scene m_Scene;
};
