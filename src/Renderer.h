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

    void OnWindowResize(unsigned int width, unsigned int height);
    void OnKeyPressed(int keycode, bool repeat);
    void OnKeyReleased(int keycode);
    void OnMouseMoved(float x, float y);
    void RestartMouse();
private:
    float m_Phi = 1.032f, m_Theta = 0.695f;

    float m_ClearColor[3] = {0.0f, 0.0f, 0.0f};

    glm::vec3 m_SunCol = glm::vec3(0.85f, 0.87f, 0.75f);
    glm::vec3 m_SkyCol = glm::vec3(0.07f, 0.08f, 0.15f);
    glm::vec3 m_RefCol = glm::vec3(0.15f, 0.06f, 0.06f);
    
    float m_Shininess = 32.0f;

    unsigned int m_WindowWidth, m_WindowHeight;

    bool m_ShowTerrainMenu = false, m_ShowBackgroundMenu = false,
         m_ShowLightMenu = false, m_ShowShadowMenu = false,
         m_ShowCamMenu = false, m_Shadows = true;
     
    bool m_Wireframe = false;

    Shader m_ShadedShader, m_WireframeShader;
    FPCamera m_Camera;
    glm::vec3 m_LastPos = glm::vec3(0.0f);

    glm::mat4 m_MVP = glm::mat4(1.0f);

    TerrainRenderer m_Terrain;
    MapRenderer m_Map;
};
