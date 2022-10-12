#pragma once

#include "subrenderers/MapRenderer.h"
#include "subrenderers/TerrainRenderer.h"
#include "Camera.h"

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
    float m_Phi = 1.032f, m_Theta = 0.695f;

    float m_ClearColor[3] = {0.2f, 0.2f, 0.2f};

    unsigned int m_WindowWidth, m_WindowHeight;

    bool m_ShowTerrainMenu = false, m_ShowBackgroundMenu = false,
         m_ShowLightMenu = false;
     
    bool m_Wireframe = false;

    Shader m_ShadedShader, m_WireframeShader;
    FPCamera m_Camera;

    glm::mat4 m_MVP = glm::mat4(1.0f);

    TerrainRenderer m_Terrain;
    MapRenderer m_Map;
};
