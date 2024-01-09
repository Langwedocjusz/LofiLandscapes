#pragma once

#include "subrenderers/Clipmap.h"
#include "subrenderers/MapGenerator.h"
#include "subrenderers/MaterialGenerator.h"
#include "subrenderers/TerrainRenderer.h"
#include "subrenderers/GrassRenderer.h"
#include "subrenderers/SkyRenderer.h"

#include "Camera.h"
#include "Serializer.h"
#include "ResourceManager.h"

#include "glad/glad.h"

class Renderer {
public:
    Renderer(uint32_t width, uint32_t height);
    ~Renderer();

    struct StartSettings {
        int Subdivisions = 64;
        int LodLevels = 7;
        int HeightRes = 4096;
        int ShadowRes = 2048;
        int MaterialRes = 1024;
        int WrapType = GL_CLAMP_TO_BORDER;
        float InternalResScale = 1.0f;
    };

    void InitImGuiIniHandler();
    void Init(StartSettings settings);

    void OnUpdate(float deltatime);
    void OnRender();
    void OnImGuiRender();

    void OnWindowResize(uint32_t width, uint32_t height);
    void OnKeyPressed(int keycode, bool repeat);
    void OnKeyReleased(int keycode);
    void OnMouseMoved(float x, float y);
    void OnMousePressed(int button, int mods);
    void RestartMouse();
private:

    bool m_Wireframe = false;

    //Show menu window flags
    //To-do: In practice using this is somewhat ugly, 
    // may switch to map<string, bool> or something like that
    bool m_ShowTerrainMenu = true,
         m_ShowLightMenu   = true,  m_ShowShadowMenu      = true,
         m_ShowCamMenu     = true,  m_ShowMaterialMenu    = true,
         m_ShowSkyMenu     = true,  m_ShowMapMaterialMenu = true,
         m_ShowGrassMenu   = false; 

    bool m_ShowTexBrowser = false, m_ShowProfiler = false;

    uint32_t m_WindowWidth, m_WindowHeight;
    float m_Aspect, m_InvAspect;
    
    FPCamera m_Camera;
    glm::vec3 m_LastPos = glm::vec3(0.0f);

    ResourceManager m_ResourceManager;
    Serializer m_Serializer;

    MapGenerator m_Map;
    MaterialGenerator m_Material;
    TerrainRenderer m_TerrainRenderer;
    GrassRenderer m_GrassRenderer;
    SkyRenderer m_SkyRenderer;

    float m_InternalResScale = 1.0f;
    uint32_t m_InternalWidth, m_InternalHeight;
    FramebufferTexture m_Framebuffer;
    Quad m_Quad;
    std::shared_ptr<VertFragShader> m_PresentShader;
};
