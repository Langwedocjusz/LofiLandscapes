#pragma once

#include "subrenderers/MapRenderer.h"
#include "subrenderers/ClipmapRenderer.h"
#include "subrenderers/MaterialRenderer.h"
#include "subrenderers/SkyRenderer.h"

#include "Camera.h"

#include "glad/glad.h"

class Renderer {
public:
    Renderer(unsigned int width, unsigned int height);
    ~Renderer();

    struct StartSettings {
        int Subdivisions = 64;
        int LodLevels = 5;
        int HeightRes = 4096;
        int ShadowRes = 4096;
        int WrapType = GL_CLAMP_TO_BORDER;
    };

    void Init(StartSettings settings);
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

    float m_SunCol[3] = {0.90f, 0.85f, 0.70f};

    float m_SunStr = 2.0f, m_SkyDiff = 0.125f, m_SkySpec = 0.175f, m_RefStr = 0.100f;
    
    float m_TilingFactor = 32.0f, m_NormalStrength = 0.25f;

    bool m_Wireframe = false, m_Shadows = true;
    bool m_Materials = true, m_FixTiling = true;
    bool m_Fog = false;

    //Show menu window flags
    bool m_ShowTerrainMenu = true, m_ShowBackgroundMenu  = false,
         m_ShowLightMenu   = true, m_ShowShadowMenu      = true,
         m_ShowCamMenu     = true, m_ShowMaterialMenu    = true,
         m_ShowSkyMenu     = true, m_ShowMapMaterialMenu = true;
    
    //"Backend oriented" things

    unsigned int m_WindowWidth, m_WindowHeight;
    float m_Aspect, m_InvAspect;
    
    Shader m_ShadedShader, m_WireframeShader;
    FPCamera m_Camera;
    glm::vec3 m_LastPos = glm::vec3(0.0f);

    glm::mat4 m_MVP = glm::mat4(1.0f);

    ClipmapRenderer m_Clipmap;
    MapRenderer m_Map;
    MaterialRenderer m_Material;
    SkyRenderer m_Sky;
};
