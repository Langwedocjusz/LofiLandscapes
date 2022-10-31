#pragma once

#include "subrenderers/MapRenderer.h"
#include "subrenderers/ClipmapRenderer.h"
#include "subrenderers/MaterialRenderer.h"
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
    //Shading settings:
    float m_Phi = 1.032f, m_Theta = 1.050f;

    float m_ClearColor[3] = {0.0f, 0.0f, 0.0f};

    glm::vec3 m_SunCol = glm::vec3(0.90f, 0.85f, 0.70f);
    glm::vec3 m_SkyCol = glm::vec3(0.06f, 0.08f, 0.25f);
    glm::vec3 m_RefCol = glm::vec3(0.15f, 0.06f, 0.06f);
    
    float m_TilingFactor = 32.0f, m_NormalStrength = 0.25f;

    bool m_Shadows = true, m_Materials = true, m_FixTiling = true;     
    
    bool m_Wireframe = false;

    //Show menu window flags
    bool m_ShowTerrainMenu = true, m_ShowBackgroundMenu = true,
         m_ShowLightMenu   = true, m_ShowShadowMenu     = true,
         m_ShowCamMenu     = true, m_ShowMaterialMenu   = true; 
    
    //"Backend oriented" things
    unsigned int m_WindowWidth, m_WindowHeight;
    
    Shader m_ShadedShader, m_WireframeShader;
    FPCamera m_Camera;
    glm::vec3 m_LastPos = glm::vec3(0.0f);

    glm::mat4 m_MVP = glm::mat4(1.0f);

    ClipmapRenderer m_Clipmap;
    MapRenderer m_Map;
    MaterialRenderer m_Material;
};
